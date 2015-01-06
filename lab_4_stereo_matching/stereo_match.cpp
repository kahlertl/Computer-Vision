#include <iostream>

#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;


static float matchSSD(const Mat& patch, const Mat& image, const Point2i center,
                      const int max_disparity, float* best_match, float* disparity)
{
    // integer division cuts off any fractional part
    int radius = patch.cols / 2;

    // normalize the row that will be entered for SSD calculation.
    int row = center.x - radius;
    int col = center.y - radius;

    // we do not want to cast rapidly from int to float, because the SSD is an
    // integer
    int best_ssd = 0;
    int best_shift = 0;

    int ssd_left = 0;
    int ssd_right = 0;

    for (int offset = 0; offset < max_disparity; offset++) {

        // walk through the patch
        for (int prow = 0; prow < patch.rows; prow++) {
            for (int pcol = 0; pcol < patch.cols; pcol++) {
                // grayscale images => uchar
                int diff_right = patch.at<uchar>(prow, pcol) - image.at<uchar>(prow + row, pcol + col + offset);
                int diff_left  = patch.at<uchar>(prow, pcol) - image.at<uchar>(prow + row, pcol + col - offset);

                ssd_right += diff_right * diff_right;
                ssd_left += diff_left * diff_left;
            }
        }

        if (ssd_left > best_ssd) {
            best_ssd    = ssd_left;
            best_shift  = -offset;
        } else if (ssd_right > best_ssd) {
            best_ssd    = ssd_left;
            best_shift  = -offset;
        }

    }


    *best_match = (float) best_ssd;
    *disparity  = (float) best_shift;
}


static void blockMatch(const Mat& left, const Mat& right, Mat& disparity,
                       const int radius, const int max_disparity)
{
    disparity = Mat::zeros(left.rows, right.rows, DataType<float>::type);

    // walk through the left image
    for (int lrow = radius; lrow < left.rows - radius; lrow++) {

        // cout << "Row " << lrow << endl;

        for (int lcol = radius; lcol < left.cols - radius; lcol++) {

            cout << lrow << ", " << lcol << endl;

            Mat patch (left,
                       Range(lrow - radius, lrow + radius),
                       Range(lcol - radius, lcol + radius));

            // imshow("Patch", patch);
            // waitKey(3000);

            float shift = 0;
            float best_match = 0;
            float match = 0;

            matchSSD(patch, left, Point2i(lcol, lrow), max_disparity, &match, &shift);

            if (match > best_match) {
                best_match = match;
                disparity.at<float>(lrow, lcol) = shift;                
            }


        }
    }
}


int main(int argc, char const *argv[])
{
    Mat img_left;
    Mat img_right;
    Mat disparity;

    if (argc != 3) {
        cout << "Usage: ./stereo_match left right" << endl;

        return 1;
    }

    // load images
    img_left  = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    img_right = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (img_left.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    if (img_right.empty()) {
        cerr << "Error cannot read " << argv[2] << endl;

        return 1;
    }

    blockMatch(img_left, img_right, disparity, 50, 5);


    return 0;
}