#include <iostream>
#include <getopt.h> // getopt_long()
#include <limits>   // numeric_limits

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

static void usage()
{
    cout << "Usage: ./stereo_match [options] left right" << endl;
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
                    const int window_size, const int max_disparity)
{
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

                    double cost = costs_prev[k_prev] + 
                                  3 * (255.0 * 255.0) * (window_size * window_size) / 15.0 * transitionCost(k, k_prev);



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


int main(int argc, char const *argv[])
{
    Mat left;
    Mat right;
    Mat disparity;

    // Argument parsing
    if (argc != 3) {
        usage();

        return 1;
    }

    left  = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    right = imread(argv[2], CV_LOAD_IMAGE_COLOR);

    if (left.empty()) {
        cerr << "Cannot read the left image: " << argv[1] << endl;

        return 1;
    }

    if (right.empty()) {
        cerr << "Cannot read the right image: " << argv[2] << endl;

        return 1;
    }

    disparity = Mat(left.size(), CV_8UC1);

    calcDisparity(left, right, disparity, 5, 15);

    normalize(disparity, disparity, 0, 255, NORM_MINMAX);
    imshow("Disparity", disparity);
    waitKey(0);

    return 0;
}