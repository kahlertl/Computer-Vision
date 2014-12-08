#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    Mat src;
    Mat dst;

    if (argc != 2) {
        cout << "Usage: ./histo_match image" << endl;

        return 1;
    }

    // Load image
    src = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE);

    if( !src.data ) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    // Separate the image in 3 places (B, G and R)
    // vector<Mat> bgr_planes;
    // split(src, bgr_planes);

    // Establish the number of bins
    int hist_size = 256;

    // Set the ranges (for B,G,R) )
    const float  range[]    = { 0, 256 } ;
    const float* hist_range = { range };

    const bool uniform    = true;
    const bool accumulate = false;

    Mat hist;

    // compute the histogram
    calcHist(&src, 1, 0, Mat(), hist, 1, &hist_size, &hist_range, uniform, accumulate);

    // draw the histograms for B, G and R
    int hist_width  = 512;
    int hist_height = 400;
    int bin_width   = cvRound((double) hist_width / hist_size);

    Mat hist_image(hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));

    // normalize the result to [ 0, hist_image.rows ]
    normalize(hist, hist, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());

    for(int i = 1; i < hist_size; i++) {
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hist.at<float>(i - 1))),
            Point(bin_width * (i),     hist_height - cvRound(hist.at<float>(i)) ),
            Scalar(225, 225, 225),
            2, 8, 0
        );
    }

    // apply histogram equalization
    equalizeHist(src, dst);

    // display
    namedWindow("Source", CV_WINDOW_AUTOSIZE);
    imshow("Source", src);

    namedWindow("Histogram", CV_WINDOW_AUTOSIZE);
    imshow("Histogram", hist_image);

    namedWindow("Histogram Equalization", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Equalization", dst);

    waitKey(0);

    return 0;
}