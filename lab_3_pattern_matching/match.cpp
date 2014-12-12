#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

Mat image;
Mat canvas;
Mat pattern;
Mat correlation;

// pattern image - pattern mean
Mat pattern_norm;
// sum((pattern_norm)^2)
float sqsum_pattern_norm = 0;

// pattern.rows * pattern.cols
float pattern_area;

int suppress_threshold = 35;
int radius = 3;

int max_row;
int max_col;

inline float mean_offset(const int offset_row, const int offset_col)
{
    float m = 0;

    for (int row = 0; row < pattern.rows; row++) {
        for (int col = 0; col < pattern.cols; col++) {
            m += image.at<uchar>(row + offset_row, col + offset_col);
        }
    }

    return m / pattern_area;
}


inline float cross_correlation(const int row, const int col)
{
    float numerator         = 0;
    float sqsum_window_norm = 0;
    float window_mean       = mean_offset(row, col);

    for (int prow = 0; prow < pattern.rows; prow++) {
        for (int pcol = 0; pcol < pattern.cols; pcol++) {
            float window_norm = image.at<uchar>(row + prow, col + pcol) - window_mean;


            numerator         += pattern_norm.at<float>(prow, pcol) * window_norm;
            sqsum_window_norm += window_norm * window_norm;

            // if (numerator != numerator) {
            //     cout << "window_norm:  " << window_norm << endl;
            //     cout << "pattern_norm: " << pattern_norm.at<float>(prow, pcol) << endl;
            //     cout << "numerator:    " << numerator << endl;
            //     cout << "sqsum_window_norm: " << sqsum_window_norm << endl;

            //     waitKey(0);
            // }

        }
    }

    float r = numerator / sqrt(sqsum_pattern_norm * sqsum_window_norm);


    // cout << "numerator: " << numerator << endl;
    // cout << "denumerator: " << sqrt(sqsum_pattern_norm * sqsum_window_norm) << endl;
    // cout << "cross_correlation: " << r << endl;
    // waitKey(0);

    return r;
    // return numerator / sqrt(sqsum_pattern_norm * sqsum_window_norm);
}

void non_maximum_suppression(Mat &matrix, const int suppress_radius, float &matrix_max)
{
    int window_size = 2 * suppress_radius + 1;

    for (int row = 0; row < matrix.rows - window_size; row++) {
        for (int col = 0; col < matrix.cols - window_size; col++) {
            float max = 0;

            for (int i = 0; i < window_size; i++) {
                for (int j = 0; j < window_size; j++) {
                    if (matrix.at<float>(row + i, col + j) > max) {
                        max = matrix.at<float>(row + i, col + j);
                    }
                }
            }

            if (max > matrix_max) {
                matrix_max = max;
            }

            for (int i = 0; i < window_size; i++) {
                for (int j = 0; j < window_size; j++) {
                    if (matrix.at<float>(row + i, col + j) < max) {
                        matrix.at<float>(row + i, col + j) = 0;
                    }
                }
            }
        }
    }

}

void on_trackbar(int, void*)
{
    float max = 0;
    int matches = 0;

    Mat suppressed_correlation;
    correlation.copyTo(suppressed_correlation);

    // cout << "Non-Maximum Suppression ... " << endl;
    non_maximum_suppression(suppressed_correlation, radius, max);

    // Clear canvas
    cvtColor(image, canvas, CV_GRAY2RGB);

    for (int row = 0; row < max_row; row++) {
        for (int col = 0; col < max_col; col++) {
            if (suppressed_correlation.at<float>(row, col) / max > (suppress_threshold / 100.)) {
                matches++;

                rectangle(
                    canvas,
                    Point(col, row),
                    Point(col + pattern.cols, row + pattern.rows),
                    {0, 0, 255}
                );
            }
        }
    }

    cout << matches << " matches" << endl;
    cout << "    radius:    " << radius << endl;
    cout << "    threshold: " << suppress_threshold << endl;
    cout << endl;

    imshow("Suppressed correlation", suppressed_correlation);
    imshow("Pattern matching", canvas);
}


int main(int argc, char const *argv[])
{
    if (argc != 3) {
        cout << "Usage: ./correlation pattern image" << endl;

        return 1;
    }

    // load image
    pattern = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    image = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (pattern.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    if (image.empty()) {
        cerr << "Error cannot read " << argv[2] << endl;

        return 1;
    }

    max_row = image.rows - pattern.rows;
    max_col = image.cols - pattern.cols;

    correlation = Mat::zeros(image.rows - pattern.rows, image.cols - pattern.cols, DataType<float>::type);
    pattern_norm = Mat::zeros(pattern.size(), DataType<float>::type);

    pattern_area = pattern.rows * pattern.cols;

    // the mean() function returns a Scalar for 4 channels
    // (B, G, R, alpha). In a gray image, there is only
    // one channel.
    float pattern_mean = mean(pattern)[0];

    for (int row = 0; row < pattern.rows; row++) {
        for (int col = 0; col < pattern.cols; col++) {
            float norm = pattern.at<uchar>(row, col) - pattern_mean;

            pattern_norm.at<float>(row, col) = norm;
            sqsum_pattern_norm              += norm * norm;
        }
    }

    cout << "Calculate correlation ... " << endl;

    for (int row = 0; row < max_row; row++) {
        for (int col = 0; col < max_col; col++) {
            correlation.at<float>(row, col) = cross_correlation(row, col);
        }
    }

    imshow("Cross correlation", correlation);

    // display
    namedWindow("Pattern matching", 1);
    createTrackbar("radius",    "Pattern matching", &radius,               5, on_trackbar);
    createTrackbar("threshold", "Pattern matching", &suppress_threshold, 100, on_trackbar);
    imshow("Pattern matching", image);
    // imshow("Pattern", pattern);

    // initial rendering
    on_trackbar(0, 0);

    waitKey(0);

    return 0;
}