#include <iostream>
#include <iomanip>  // std::setw
#include <stack>
#include <getopt.h> // getopt_long()
#include <limits>   // numeric_limits

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

static const int none = -1;



typedef struct Node {
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

class Tree
{
  public:
    std::vector<Node> nodes;

    // index of the root node
    int root = none;

    Tree(const int rows, const int cols)
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
                }
                nodes[i] = node;
            }
        }
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

    friend ostream& operator<<(ostream& os, const Tree& tree);
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


static void usage()
{
    cout << "Usage: ./stereo_match [options] left right"                                         << endl
         << "  options:"                                                                         << endl
         << "    -h, --help            Show this help message"                                   << endl
         << "    -w, --window-size     Size of the windows used for stereo matching. Default: 5" << endl
         << "    -d, --max-disparity   Shrinks the range that will be used"                      << endl
         << "                          for block matching. Default: 20"                          << endl
         << "    -t, --target          Name of output file. Default: disparity.png"              << endl
         << "    -s, --scale-cost      Scaling factor for the cost function for different"       << endl
         << "                          pixels. Default: 0.075"                                   << endl;
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

            // cout << "(" << (row + offset_row) << "," << (col_left   - offset_col) << ")" << endl;
            // cout << "(" << (row + offset_row) << "," << (col_right  - offset_col) << ")" << endl;

            const Vec3b pixel_left  = left.at<Vec3b> (row + offset_row, col_left   - offset_col);
            const Vec3b pixel_right = right.at<Vec3b>(row + offset_row, col_right  - offset_col);

            distance += pow(pixel_left[1] - pixel_right[1], 2) +
                        pow(pixel_left[2] - pixel_right[2], 2) +
                        pow(pixel_left[3] - pixel_right[3], 2);
        }
    }

    return distance;
}


int transitionCost(const int x, const int y)
{
    return abs(x - y);
}


void calcDisparity(const Mat& left, const Mat& right, Mat& disparity,
                    const int window_size, const int max_disparity, const double cost_factor)
{
    // scale cost function:
    // 
    //    (max value of SSD color match) / max disparity * cost_factor
    //    
    const double cost_scale = 3 * (255.0 * 255.0) * (window_size * window_size) / max_disparity * cost_factor;

    // initialize disparity map matrix as a grayscale image
    disparity = Mat(left.size(), CV_8UC1);

    for (int row = 0; row < left.rows - window_size; row++) {
        cout << "." << flush;
        
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
            // cout << "(" << row << "," << col << ")" << endl;

            for (int k = 0; k < max_disparity; k++) {
                // cout << "k = " << k << endl;

                // compute next node with minimum costs
                double min = numeric_limits<double>::max();

                for (int k_prev = 0; k_prev < max_disparity; k_prev++) {
                    // cout << "k_prev = " << k_prev << endl;

                    // cout << "transitionCost = " << transitionCost(k, k_prev) << endl;

                    double cost = costs_prev[k_prev] + cost_scale * transitionCost(k, k_prev);



                    // a better minimum was found
                    if (cost < min) {
                        // update minimum
                        min = cost;
                        // store the best predecessor
                        path_pointers[col][k] = k_prev;
                    }
                }

                double cost = matchSSDColor(left, right, window_size, row, col, col - k) + min;
                // cout << "min = " << min << ", cost = " << cost << endl;
                
                // updates the
                costs_current[k] = cost;
            }

            // cout << "previous costs:" << endl;
            // for (int i = 0; i < costs_current.size(); i++) {
            //     cout << "  " << costs_current[i] << endl;
            // }

            // copy costs for the next pixel
            costs_prev = costs_current;

            // cin.get();
        }


        // Backward pass
        // 

        // find minimal node in the very last column
        double min = numeric_limits<double>::max();

        for (int k = 0; k < max_disparity; k++) {
            if (costs_current[k] < min) {
                // update minimum
                min = costs_current[k];
                disparity.at<uchar>(row, left.cols - 1) = (uchar) k;
            }
        }

        // use the stored pointers to get the minimal path
        for (int col = left.cols - 2; col >= 0; col--) {
            disparity.at<uchar>(row, col) = (uchar) path_pointers[col + 1][disparity.at<uchar>(row, col + 1)];
        }
    }
    cout << endl;
}


void calcDisparity(const Mat& left, const Mat& right, Tree& tree, Mat& disparity,
                   const int window_size, const int max_disparity, const double cost_factor)
{
    // scale cost function:
    // 
    //    (max value of SSD color match) / max disparity * cost_factor
    // 
    const double cost_scale = 3 * (255.0 * 255.0) * (window_size * window_size) / max_disparity * cost_factor;
    
    // initialize disparity map matrix as a grayscale image
    disparity = Mat(left.size(), CV_8UC1);


    vector<vector<int>> path_pointers(tree.nodes.size(), vector<int>(max_disparity));

    // Stores the cost of the path to each previous nodes
    // We cannot write the costs directly back into the costs_prev vector
    // because the previous costs are requied in the next iteration step
    // of k_prev again.
    vector<double> costs_prev(max_disparity, 0); // = F_{i - 1}
    vector<double> costs_current(max_disparity); // = F_i

    // depth first search approach
    stack<int> nodes_stack;
    nodes_stack.push(tree.root);

    // Forward path
    // 
    while (!nodes_stack.empty()) {
        const int i = nodes_stack.top();
        nodes_stack.pop();

        // Node node  = tree.nodes[i];
        Node& node = tree.nodes[i];
        int row    = i / left.cols;
        int col    = i % left.cols;
        
        // if the window would overflow the image at the current position
        // skip this node
        if (row >= left.rows - window_size) { continue; }
        if (col < window_size)              { continue; }

        // where to go?
        // cout << setw(2) << i << ": (" << row << "," << col << ")" <<  endl;

        if (node.children[1] != none) { nodes_stack.push(node.children[1]); }
        if (node.children[0] != none) { nodes_stack.push(node.children[0]); }
        if (node.children[2] != none) { nodes_stack.push(node.children[2]); }

        for (int k = 0; k < max_disparity && k <= col; k++) {
            // cout << "k = " << k << endl;

            // compute next node with minimum costs
            double min = numeric_limits<double>::max();

            for (int k_prev = 0; k_prev < max_disparity; k_prev++) {
                // cout << "k_prev = " << k_prev << endl;

                // cout << "transitionCost = " << transitionCost(k, k_prev) << endl;

                double cost = costs_prev[k_prev] + cost_scale * transitionCost(k, k_prev);

                // a better minimum was found
                if (cost < min) {
                    // update minimum
                    min = cost;
                    // store the best predecessor
                    path_pointers[i][k] = k_prev;
                }
            }

            double cost = matchSSDColor(left, right, window_size, row, col, col - k) + min;
            // cout << "min = " << min << ", cost = " << cost << endl;
            
            costs_current[k] = cost;
        }

        if (node.numChildNodes() > 1 && node.costs == nullptr) {
            node.costs = new vector<double>(costs_current);
        }

        // Leaf. Load the previous costs from the parent of the next node
        if (node.numChildNodes() == 0) {
            Node& next_node = tree.nodes[nodes_stack.top()];
            costs_prev      = *(tree.nodes[next_node.parent].costs);
        } else {
            // copy costs for the next pixel
            costs_prev = costs_current;
        }



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
    string target         = "disparity.png";


    const struct option long_options[] = {
        { "help",           no_argument,       0, 'h' },
        { "window-size",    required_argument, 0, 'w' },
        { "target",         required_argument, 0, 't' },
        { "max-disparity",  required_argument, 0, 'd' },
        { "scale-cost",     required_argument, 0, 's' },
        0 // end of parameter list
    };

    // parse command line options
    while (true) {
        int index  = -1;
        int result = getopt_long(argc, (char **) argv, "hw:t:d:s:", long_options, &index);

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

            // target - filename of the output disparity map
            case 't':
                target = optarg;
                break;

            // scale factor of the cost function
            case 's':
                cost_scale = stof(string(optarg));
                if (cost_scale <= 0) {
                    cerr << argv[0] << ": Invalid scale factor for cost function: " << optarg << endl;
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

    Tree tree(left.rows, left.cols);
    // cout << tree;

    for (int i = 0; i < tree.nodes.size(); ++i) {
        Node& node = tree.nodes[i];
    }

    // calcDisparity(left, right, disparity, window_size, max_disparity, cost_scale);
    calcDisparity(left, right, tree, disparity, window_size, max_disparity, cost_scale);

    // // normalize disparity to a regular grayscale image
    // normalize(disparity, disparity, 0, 255, NORM_MINMAX);

    // try {
    //     imwrite(target, disparity);
    // } catch (runtime_error& ex) {
    //     cerr << "Error: cannot save disparity map to '" << target << "'" << endl;

    //     return 1;
    // }

    return 0;
}