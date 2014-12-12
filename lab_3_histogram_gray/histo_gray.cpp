#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

#include "histogram.h"

using namespace std;
using namespace cv;


int main(int argc, char** argv)
{
    Mat image;
    Mat equalized;
    Mat pattern;

    if (argc != 3) {
        cout << "Usage: ./histo_match image pattern" << endl;

        return 1;
    }

    // Load image
    image   = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    pattern = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (!image.data ) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    if (!pattern.data ) {
        cerr << "Error cannot read " << argv[2] << endl;

        return 1;
    }

    // Establish the number of bins
    int hist_size = 256;

    // Set the ranges (for B,G,R) )
    const float  range[]    = { 0, 256 } ;
    const float* hist_range = { range };

    const bool uniform    = true;
    const bool accumulate = false;

    Mat hist_image;
    Mat hist_pattern;

    // compute the histograms
    calcHist(&image,   1, 0, Mat(), hist_image,   1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&pattern, 1, 0, Mat(), hist_pattern, 1, &hist_size, &hist_range, uniform, accumulate);

    // compute cumulative distribution function
    Mat cdf_image;
    Mat cdf_pattern;

    cumsum(hist_image,   cdf_image);
    cumsum(hist_pattern, cdf_pattern);
    

    // canvas for histograms and CDFs
    int hist_width  = 512;
    int hist_height = 400;

    Mat canvas_hist(hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));

    draw_histogram(hist_image,  canvas_hist, {  0,   0, 255});
    draw_histogram(cdf_image ,  canvas_hist, {255,   0,   0});
    draw_histogram(cdf_pattern, canvas_hist, {  0, 255,   0});

    // apply histogram equalization
    equalizeHist(image, equalized);

    // display
    namedWindow("Source", CV_WINDOW_AUTOSIZE);
    imshow("Source", image);

    namedWindow("Histogram and CDFs", CV_WINDOW_AUTOSIZE);
    imshow("Histogram and CDFs", canvas_hist);

    namedWindow("Histogram Equalization", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Equalization", equalized);

    histogram_matching(image, pattern, image);

    namedWindow("Histogram Matching", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Matching", image);

    waitKey(0);

    return 0;
}