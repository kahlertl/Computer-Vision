#include <iostream>
#include <iomanip>  // std::setw
#include <stack>
#include <getopt.h> // getopt_long()
#include <limits>   // numeric_limits
#include <assert.h> // assert
#include <tuple>    // std::tuple, std::tie

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

// neat shortcut for node indices representing a none existent
// node reference
static const int none = -1;
int conversion_offset = 0;

// signature of all cost functions
typedef int (*cost_t)(const int a, const int b);

/**
 * Very simple and small node implementation that is used in used by the Tree
 * class below.
 */
typedef struct Node
{
    int parent;
    int children[3]; // left, middle, right

    vector<double>* costs;

    // initialize all node indices with none
    Node() : parent(none), children { none, none, none }, costs(nullptr) { };

    inline int const numChildNodes()
    {
        int num = 0;

        if (children[0] != none) { num++; }
        if (children[1] != none) { num++; }
        if (children[2] != none) { num++; }

        return num;
    }
} Node;

/**
 * A simple class that represents the tree used for the Markov tree. The tree
 * is represented as a list of nodes, where the index of a node is the running
 * number in the matrix:
 *
 *     --------------------------
 *     |  0 |  1 |  2 |  3 |  4 |
 *     --------------------------
 *     |  5 |  6 |  7 |  8 |  9 |
 *     --------------------------
 *     | 10 | 11 | 12 | 13 | 14 |
 *     --------------------------
 *     | 15 | 16 | 17 | 18 | 19 |
 *     --------------------------
 *     | 20 | 21 | 22 | 23 | 24 |
 *     --------------------------
 *
 * The nodes holds the indices of their parent or child nodes. You can
 * map an index bijective to the related (row,col) tuple:
 *
 *     row = i / grid_width
 *     col = i % grid_width
 * 
 * The tree has the following structure:
 *

 *    ○--○--○--○--○
 *          |
 *    ○--○--○--○--○
 *          |
 *    ○--○--○--○--○
 *          |
 *    ○--○--○--○--○
 *          |
 *    ○--○--○--○--○
 * 
 */
class Tree
{
  public:
    std::vector<Node> nodes;
    std::vector<int>  leafs; // indices of the nodes that are leafs
    int               root;  // index of the root node

    Tree(const int rows, const int cols) : root(none)
    {
        const int middle = cols / 2;

        // initialize tree hierarchy with number of elements in the matrix
        nodes = vector<Node>(rows * cols);

        // generate grid spanning tree
        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                const int i = row * cols + col;
                Node node;
                
                // left
                if (col < middle) {
                    node.parent   = row * cols + col + 1; // node to the right is parent
                    // node to the left
                    if (col > 0) {
                        node.children[0] = row * cols + col - 1;
                    }
                    // there is no right child, therefore the node is a leaf
                    else {
                        leafs.push_back(i);
                    }
                }
                // middle
                else if (col == middle) {
                    // check if the parent nodes above exists
                    if (row > 0) {
                        node.parent = (row - 1) * cols + col;
                    }
                    // if not, we have found the root node
                    else {
                        root = i;
                    }

                    // left and right child nodes
                    node.children[0] = row * cols + col - 1;
                    node.children[2] = row * cols + col + 1;
                    // check if the node below exists
                    if (row + 1 < rows) {
                        node.children[1] = (row + 1) * cols + col;
                    }
                }
                // right 
                else {
                    node.parent = row * cols + col - 1;

                    // node the the right is child
                    if (col + 1 < cols) {
                        node.children[2] = row * cols + col + 1;
                    }
                    // there is no right child, therefore the node is a leaf
                    else {
                        leafs.push_back(i);
                    }
                }
                nodes[i] = node;
            }
        }

        assert(root != none);
    }

    ~Tree()
    {
        // cout << "Destruct Tree ..." << endl;

        // delete all old costs
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].costs != nullptr) {
                // cout << "Delete " << i << endl;
                delete nodes[i].costs;
            }
        }
    };

    // some neat shortcuts
    inline const Node& operator[] (const int i) const { return nodes[i];     }
    inline       Node& operator[] (const int i)       { return nodes[i];     }
    inline const int   size()                         { return nodes.size(); }

    /**
     * Descide for a node if all child nodes have already be calculated.
     * 
     * @return true if the costs for all child nodes are already calculated
     */
    inline bool canCalculate(int i)
    {
        const Node& node = nodes[i];

        if (node.children[0] != none && nodes[node.children[0]].costs == nullptr) { return false; }
        if (node.children[1] != none && nodes[node.children[1]].costs == nullptr) { return false; }
        if (node.children[2] != none && nodes[node.children[2]].costs == nullptr) { return false; }

        return true;
    }
};


ostream& operator<<(ostream& os, const Tree& tree)
{
    for (int i = 0; i < tree.nodes.size(); i++) {
        os << setw(2) << i                         << ": "
           << setw(3) << tree.nodes[i].parent      << " | "
           << setw(2) << tree.nodes[i].children[0] << " "
           << setw(2) << tree.nodes[i].children[1] << " "
           << setw(2) << tree.nodes[i].children[2] << endl;
    }

    return os;
}


/**
 * The tree is stored as a list. The nodes have stores the specific indices of their parent or child nodes.
 * You can convert an index back into an (row, col) if you now the image width and the window size.
 *
 * The windows size is important, because we crop a vertical strip on the left side of the image. This
 * is required because we use block matching.
 */
inline tuple<int, int> convertIndex(const int index, const int image_width)
{
    return make_tuple(
        index / (image_width - conversion_offset),                    // row
        index % (image_width - conversion_offset) + conversion_offset // col
    );
}


static void usage()
{
    cout << "Usage: ./stereo_match [options] left right"                                         << endl
         << "  options:"                                                                         << endl
         << "    -h, --help            Show this help message"                                   << endl
         << "    -w, --window-size     Size of the windows used for stereo matching. Default: 5" << endl
         << "    -d, --max-disparity   Shrinks the range that will be used"                      << endl
         << "                          for block matching. Default: 20"                          << endl
         << "    -o, --output          Name of output file. Default: disparity.png"              << endl
         << "    -s, --scale-cost      Scaling factor for the cost function for different"       << endl
         << "                          pixels. Default: 0.075"                                   << endl
         << "    -c, --cost            Cost function for the transition of two disparity values" << endl
         << "                            Available:"                                             << endl
         << "                              - potts"                                              << endl
         << "                              - abs_diff"                                           << endl
         << "                              - square_diff"                                        << endl
         << "                          Default: abs_diff"                                        << endl
         << "    -t, --topology        Defines the topology that is used for the Markov model."  << endl
         << "                            Available:"                                             << endl
         << "                              - tree"                                               << endl
         << "                              - line"                                               << endl
         << "                          Default: tree"                                            << endl;
}


static bool parsePositionalImage(Mat& image, const int channels, const string& name, int argc, char const *argv[])
{
    if (optind >= argc) {
        cerr << argv[0] << ": required argument: '" << name << "'" << endl;
        usage();

        return false;
    } else {
        image = imread(argv[optind++], channels);

        if (image.empty()) {
            cerr << "Error: Cannot read '" << argv[optind] << "'" << endl;

            return false;
        }
    }

    return true;
}



double matchSSDColor(const Mat& left, const Mat& right,
                     const int window_size, const int row, const int col_left, const int col_right)
{
    double distance = 0;

    for (int offset_row = 0; offset_row < window_size; offset_row++) {
        for (int offset_col = 0; offset_col < window_size; offset_col++) {

            // cout << "left  (" << (row + offset_row) << "," << (col_left   - offset_col) << ")" << endl;
            // cout << "right (" << (row + offset_row) << "," << (col_right  - offset_col) << ")" << endl;

            const Vec3b pixel_left  = left.at<Vec3b> (row + offset_row, col_left   - offset_col);
            const Vec3b pixel_right = right.at<Vec3b>(row + offset_row, col_right  - offset_col);

            distance += pow(pixel_left[0] - pixel_right[0], 2) +
                        pow(pixel_left[1] - pixel_right[1], 2) +
                        pow(pixel_left[2] - pixel_right[2], 2);
        }
    }

    return distance;
}


int potts_cost  (const int x, const int y) { return (x != y) ? 1 : 0;  }
int abs_diff    (const int x, const int y) { return abs(x - y);        }
int square_diff (const int x, const int y) { return (x - y) * (x - y); }


void calcDisparityLine(const Mat& left, const Mat& right, Mat& disparity,
                       const int window_size, const int max_disparity, cost_t cost_fn, const double cost_factor)
{
    // scale cost function:
    // 
    //    (max value of SSD color match) / max disparity * cost_factor
    //    
    const double cost_scale = 3 * (255.0 * 255.0) * (window_size * window_size) / max_disparity * cost_factor;

    // initialize disparity map matrix as a grayscale image
    disparity = Mat(left.size(), CV_8UC1);

    for (int row = 0; row < left.rows - window_size; row++) {
        cout << "." << flush; // progress bar
        
        vector<vector<int>> path_pointers(left.cols, vector<int>(max_disparity));

        // stores the cost of the path to each previous nodes
        vector<double> costs_prev(max_disparity, 0); // = F_{i - 1}

        // We cannot write the costs directly back into the prev_costs vector
        // because the previous costs are requied in the next iteration step
        // of k_prev again.
        vector<double> costs_current(max_disparity); // = F_i

        // Forward path
        // 
        for (int col = window_size + max_disparity + 1; col < left.cols; col++) {
            for (int k = 0; k < max_disparity; k++) {
                // compute next node with minimum costs
                double min = numeric_limits<double>::max();

                for (int k_prev = 0; k_prev < max_disparity; k_prev++) {
                    double cost = costs_prev[k_prev] + cost_scale * cost_fn(k, k_prev);

                    // a better minimum was found
                    if (cost < min) {
                        min = cost;                     // update minimum
                        path_pointers[col][k] = k_prev; // store the best predecessor
                    }
                }
                costs_current[k] = matchSSDColor(left, right, window_size, row, col, col - k) + min;
            }
            // copy costs for the next pixel
            costs_prev = costs_current;
        }

        // Backward pass
        // 

        // find minimal node in the very last column
        double min = numeric_limits<double>::max();

        for (int k = 0; k < max_disparity; k++) {
            if (costs_current[k] < min) {
                min = costs_current[k]; // update minimum
                disparity.at<uchar>(row, left.cols - 1) = (uchar) k;
            }
        }

        // use the stored pointers to get the minimal path
        for (int col = left.cols - 2; col >= 0; col--) {
            disparity.at<uchar>(row, col) = (uchar) path_pointers[col + 1][disparity.at<uchar>(row, col + 1)];
        }
    }
    cout << endl; // finish progress bar
}


void calcDisparityTree(const Mat& left, const Mat& right, Tree& tree, Mat& disparity,
                       const int window_size, const int max_disparity, cost_t cost_fn, const double cost_factor)
{
    // scale cost function:
    // 
    //    (max value of SSD color match) / max disparity * cost_factor
    // 
    const double cost_scale = 3 * (255.0 * 255.0) * (window_size * window_size) / max_disparity * cost_factor;
    
    // initialize disparity map matrix as a grayscale image
    disparity = Mat(left.size(), CV_8UC1);

    vector<vector<int>> path_pointers(tree.size(), vector<int>(max_disparity));
    stack<int> node_stack;

    // populate stack with all leafs
    for (int i = 0; i < tree.leafs.size(); i++) {

        // int row    = tree.leafs[i] / (left.cols - window_size);
        // int col    = tree.leafs[i] % (left.cols - window_size) + window_size;
        // cout << "leaf (" << row << "," << col << ")" << endl;

        // initialize costs for the leaf with 0
        Node& node = tree[tree.leafs[i]];
        node.costs = new vector<double>(max_disparity, 0);

        // index of the parent node of the leaf
        const int p = tree[tree.leafs[i]].parent;

        if (tree.canCalculate(p)) {
            node_stack.push(p);

            // int row_p    = p / (left.cols - window_size);
            // int col_p    = p % (left.cols - window_size) + window_size;
            // cout << "push (" << row_p << "," << col_p << ")" << endl;
        }
    }

    // Forward path
    // 
    while (!node_stack.empty()) {
        const int i = node_stack.top();
        node_stack.pop();

        Node& node = tree[i];
        int row, col;
        tie(row, col) = convertIndex(i, left.cols);
        
        // where to go?
        // cout << setw(2) << i << ": (" << row << "," << col << ")" <<  endl;

        if (node.parent != none && tree.canCalculate(node.parent)) {
            node_stack.push(node.parent);
        }

        assert(node.costs == nullptr);
        node.costs = new vector<double>(max_disparity);

        for (int k = 0; k < max_disparity && k <= col; k++) {
            // compute next node with minimum costs
            double min = numeric_limits<double>::max();

            for (int k_prev = 0; k_prev < max_disparity; k_prev++) {

                // sum up costs of all child nodes
                double cost_prev  = 0;
                double trans_cost = cost_scale * cost_fn(k, k_prev);
                if (node.children[0] != none) { cost_prev += (*tree[node.children[0]].costs)[k_prev] + trans_cost; }
                if (node.children[1] != none) { cost_prev += (*tree[node.children[1]].costs)[k_prev] + trans_cost; }
                if (node.children[2] != none) { cost_prev += (*tree[node.children[2]].costs)[k_prev] + trans_cost; }

                // a "better" minimum was found
                if (cost_prev < min) {
                    min = cost_prev;              // update minimum
                    path_pointers[i][k] = k_prev; // store the best predecessor
                }
            }

            double cost = matchSSDColor(left, right, window_size, row, col, col - k) + min;
            
            (*node.costs)[k] = cost;
        }

        // costs of the child nodes are no longer needed, so we free it and set the pointer to null
        if (node.children[0] != none) { delete tree[node.children[0]].costs; tree[node.children[0]].costs = nullptr; }
        if (node.children[1] != none) { delete tree[node.children[1]].costs; tree[node.children[1]].costs = nullptr; }
        if (node.children[2] != none) { delete tree[node.children[2]].costs; tree[node.children[2]].costs = nullptr; }

        if (node.parent != none && tree.canCalculate(node.parent)) {
            node_stack.push(node.parent);
        }
    }

    // Backward pass
    // 

    assert(tree[tree.root].costs != nullptr);
    assert(node_stack.size() == 0);

    // find disparity with minimal costs for the root node
    double min = numeric_limits<double>::max();

    vector<double>& root_costs = *tree[tree.root].costs;

    for (int k = 0; k < max_disparity; k++) {
        // update minimum and assign disparity value if a smaller node was found
        if (root_costs[k] < min) {
            min = root_costs[k];
            int row, col;
            tie(row, col) = convertIndex(tree.root, left.cols);
            disparity.at<uchar>(row, col) = (uchar) k;
        }
    }

    // initial population of the child node stack (depth first search approach because of small
    // memory usage)
    if (tree[tree.root].children[0] != none) { node_stack.push(tree[tree.root].children[0]); }
    if (tree[tree.root].children[1] != none) { node_stack.push(tree[tree.root].children[1]); }
    if (tree[tree.root].children[2] != none) { node_stack.push(tree[tree.root].children[2]); }

    // use the stored indices to get the minimal path
    while (!node_stack.empty()) {
        int i = node_stack.top();  // index of the current node
        int p = tree[i].parent;    // index of parent node
        node_stack.pop();

        assert(p != none);

        // compute row an columns from the indices
        int row, col;
        tie(row, col)  = convertIndex(i, left.cols);

        int row_parent, col_parent;
        tie(row_parent, col_parent) = convertIndex(p, left.cols);

        // get the disparity value for the parent node
        uchar disp_parent = disparity.at<uchar>(row_parent, col_parent);

        // the pointer stores the best disparity of the predecessor node
        disparity.at<uchar>(row, col) = (uchar) path_pointers[p][disp_parent];

        // add child nodes of the current 
        if (tree[i].children[0] != none) { node_stack.push(tree[i].children[0]); }
        if (tree[i].children[1] != none) { node_stack.push(tree[i].children[1]); }
        if (tree[i].children[2] != none) { node_stack.push(tree[i].children[2]); }
    }
}


int main(int argc, char const *argv[])
{
    Mat left;
    Mat right;
    Mat disparity;

    // parameters
    int    window_size    = 5;
    int    max_disparity  = 15;
    float  cost_scale     = 0.075;
    string topology       = "tree";
    string output         = "disparity.png";
    string cost_fn_name   = "abs_diff";
    cost_t cost_fn        = &abs_diff;

    const struct option long_options[] = {
        { "help",           no_argument,       0, 'h' },
        { "window-size",    required_argument, 0, 'w' },
        { "output",         required_argument, 0, 'o' },
        { "max-disparity",  required_argument, 0, 'd' },
        { "scale-cost",     required_argument, 0, 's' },
        { "topology",       required_argument, 0, 't' },
        { "cost",           required_argument, 0, 'c' },
        0 // end of parameter list
    };

    // parse command line options
    while (true) {
        int index  = -1;
        int result = getopt_long(argc, (char **) argv, "hw:o:d:s:t:c:", long_options, &index);

        // end of parameter list
        if (result == -1) {
            break;
        }

        switch (result) {
            // help
            case 'h':
                usage();
                return 0;

            // windows size
            case 'w':
                window_size = stoi(string(optarg));
                if (window_size < 0) {
                    cerr << argv[0] << ": Invalid window_size: " << optarg << endl;
                    return 1;
                }
                break;

            // maximal disparity
            case 'd':
                max_disparity = stoi(string(optarg));
                if (max_disparity <= 0) {
                    cerr << argv[0] << ": Invalid maximal disparity: " << optarg << endl;
                    return 1;
                }
                break;

            // output - filename of the output disparity map
            case 'o':
                output = optarg;
                break;

            // scale factor of the cost function
            case 's':
                cost_scale = stof(string(optarg));
                if (cost_scale <= 0) {
                    cerr << argv[0] << ": Invalid scale factor for cost function: " << optarg << endl;
                    return 1;
                }
                break;

            // cost function for transitions
            case 'c':
                cost_fn_name = string(optarg);
                if      (cost_fn_name == "potts")       { cost_fn = &potts_cost;  }
                else if (cost_fn_name == "abs_diff")    { cost_fn = &abs_diff;    }
                else if (cost_fn_name == "square_diff") { cost_fn = &square_diff; }
                else {
                    cerr << argv[0] << ": Invalid cost function: " << optarg << endl;
                    return 1;
                }

                break;

            // topology
            case 't':
                topology = string(optarg);
                if (topology != "tree" && topology != "line") {
                    cerr << argv[0] << ": Invalid topology: " << optarg << endl;
                    return 1;
                }

                break;

           // missing option
           case '?':
                return 1;

            // unknown
            default:
                cerr << "unknown parameter: " << optarg << endl;
                break;
        }
    }

    // parse positional arguments
    if (!parsePositionalImage(left,  CV_LOAD_IMAGE_COLOR, "left",  argc, argv)) { return 1; }
    if (!parsePositionalImage(right, CV_LOAD_IMAGE_COLOR, "right", argc, argv)) { return 1; }

    // Validate parameter set
    // 
    if (left.size() != right.size()) {
        cerr << "Error: images must be of same size" << endl;
        return 1;
    }
    if (window_size >= left.cols || window_size >= left.rows) {
        cerr << "Error: Windows size must be smaller than the image" << endl;
        return 1;
    }

    cout << "Parameters:"                          << endl
         << "  window size   : " << window_size    << endl
         << "  max disparity : " << max_disparity  << endl
         << "  topology      : " << topology       << endl
         << "  cost scale    : " << cost_scale     << endl
         << "  cost function : " << cost_fn_name   << endl
         << "  output        : " << output         << endl;

    disparity = Mat::zeros(left.size(), CV_8UC1);

    if (topology == "tree") {
        conversion_offset = max_disparity;
        Tree tree(left.rows - conversion_offset, left.cols - conversion_offset);
        calcDisparityTree(left, right, tree, disparity, window_size, max_disparity, cost_fn, cost_scale);
    } else { // topology == "line"
        calcDisparityLine(left, right, disparity, window_size, max_disparity, cost_fn, cost_scale);
    }


    // normalize disparity to a regular grayscale image
    normalize(disparity, disparity, 0, 255, NORM_MINMAX);

    try {
        imwrite(output, disparity);
    } catch (runtime_error& ex) {
        cerr << "Error: cannot save disparity map to '" << output << "'" << endl;

        return 1;
    }

    return 0;
}