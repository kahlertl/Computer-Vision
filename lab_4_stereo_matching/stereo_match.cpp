#include <iostream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;


static float matchSSD(const int radius, const Mat& left, const Mat& right, const Point2i center,
                      const int max_disparity, float* best_match, int* disparity)
{
    // we do not want to cast rapidly from int to float, because the SSD is an
    // integer
    int min_ssd = INT_MAX;

    int start = -max_disparity;
    int end   =  max_disparity;
    
    if (center.y + start < 0) {
        start = -(center.y - radius);
    }
    if (center.y + end > right.cols) {
        end = right.cols - center.y;
    }

        // cout << "center.y = " << center.y << endl;
        // cout << "start    = " << start << endl;
        // cout << "center.y - start = " << (center.y - start) << endl;
        // cout << "end = " << end << endl;

    Mat patch  = Mat::zeros(2 * radius + 1, 2 * radius + 1, CV_8UC1);
    Mat search = Mat::zeros(2 * radius + 1, 2 * radius + 1, CV_8UC1);

    for (int col_offset = start; col_offset < end; col_offset += 1) {
        int ssd = 0;

        // walk through the patch
        for (int prow = -radius; prow < radius; prow++) {
            for (int pcol = -radius; pcol < radius; pcol++) {

                patch.at<uchar>(prow + radius, pcol + radius)  = left.at<uchar>(center.y + prow, center.x + pcol);
                search.at<uchar>(prow + radius, pcol + radius) = right.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                // grayscale images => uchar
                // patch - image
                int diff =    left.at<uchar>(center.y + prow, center.x + pcol)
                           - right.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                ssd  += diff * diff;
            }
        }


        // if (center.y > 200 && center.x > 120) {
        //     imshow("patch", patch);
        //     imshow("search", search);
        //     waitKey(0);

        //     cout << col_offset << ": " << ssd << endl;
        // }


        if (ssd < min_ssd) {
            min_ssd = ssd;
            *disparity = -col_offset;
        }

    }

    // if (center.y > 150) {

    //     Mat canvas;
    //     left.copyTo(canvas);

    //     for (int row = -radius; row < radius; row++) {
    //         for (int col = -radius; col < radius; col++) {
    //             // cout << row << ", " << col << endl;
    //             // canvas.at<uchar>(center.y + row, center.x + col) = left.at<uchar>(center.y + row, center.x + col);
    //             // canvas.at<uchar>(center.y + row, center.x + col) = 0;
    //             // canvas.at<uchar>(center.y + row, center.x + col) = right.at<uchar>(center.y + row, center.x + col);
    //             canvas.at<uchar>(center.y + row, center.x + col) = right.at<uchar>(center.y + row, center.x + col - (int) *disparity);
    //         }
    //     }
    //     imshow("canvas", canvas);
    //     waitKey(10);

    // }

    // cout << "disparity = " << *disparity << endl;

    // *best_match = (float) best_ssd;
}


static void blockMatch(const Mat& left, const Mat& right, Mat& disparity,
                       const int radius, const int max_disparity)
{
    disparity = Mat::zeros(left.size(), DataType<int>::type);

    // walk through the left image
    for (int lrow = radius; lrow < left.rows - radius; lrow++) {

        // cout << "Row " << lrow << endl;
        cout << "." << flush;

        for (int lcol = radius; lcol < left.cols - radius; lcol++) {

            // cout << lrow << ", " << lcol << endl;

            int shift = 0;
            // float best_match = 0;
            float match = 0;

            matchSSD(radius, left, right, Point2i(lcol, lrow), max_disparity, &match, &shift);

            // Mat patch (left,
            //            Range(lrow - radius, lrow + radius),
            //            Range(lcol - radius, lcol + radius));

            // Mat search (right,
            //             Range(lrow - radius, lrow + radius),
            //             Range(0, right.cols));

            // Mat result;

            // matchTemplate(search, patch, result, CV_TM_SQDIFF);



            // double min_val;
            // double max_val;
            // Point min_loc;

            // minMaxLoc(result, &min_val, &max_val, &min_loc);


            // // Mat canvas;
            // // search.copyTo(canvas);

            // // cv::rectangle(
            // //     canvas, 
            // //     min_loc, 
            // //     Point(min_loc.x + patch.cols, min_loc.y + patch.rows), 
            // //     CV_RGB(0,255,0),
            // //     2
            // // );
            // // imshow("search", canvas);
            // // imshow("patch", patch);
            // // waitKey(0);


            // min_loc.y += lrow;

            // shift = min_loc.x - lcol;

            // cout << "min_loc " << min_loc << endl;
            // cout << "cnt_loc [" << lcol << ", " << lrow << "]" << endl;

            // for (int row = min_loc; row < patch.rows; row++) {
            //     for (int col = min_loc; col < patch.cols; col++) {
            //         canvas.at<uchar>(row, col) = right.at<uchar>(row, col)
            //     }
            // }

            // cout << result << endl;
            // cout << "min = " << min_val << " " << min_loc << endl;
            // waitKey(0);

            // if (match > best_match) {
                // best_match = match;
                disparity.at<int>(lrow, lcol) = shift;                
            // }


        }
    }

    cout << endl;
}


/**
 * Normalizes the disparoty map to an grayscale image [0, 255]
 */
void normDisp(Mat& disparity, Mat& normalized)
{
    double minval;
    double maxval;

    // initialize matrix
    normalized = Mat::zeros(disparity.size(), CV_8UC1);

    // search minimum and maximum of the disparoty map 
    minMaxLoc(disparity, &minval, &maxval);

    // cout << "min: " << minval << ", maxval: " << maxval << endl;

    // compute normalization scaling factor to get all values into
    // range [0, 255]
    float norm = 255.0 / (maxval - minval);

    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            normalized.at<uchar>(row, col) =  norm * (disparity.at<int>(row, col) - minval);
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

    blockMatch(img_left, img_right, disparity, 16, 16);
    // cout << "disparity " << endl << disparity << endl;

    // normalize the result to [ 0, 255 ]
    Mat normalized;
    normDisp(disparity, normalized);

    imshow("Disparity", normalized);
    waitKey(0);

    return 0;
}