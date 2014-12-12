#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

#include "histogram.h"

using namespace std;
using namespace cv;


int main( int argc, char** argv )
{
    Mat image;

    if (argc != 2) {
        cout << "Usage: ./histo_color image" << endl;

        return 1;
    }

    // load image
    image = imread(argv[1], 1);

    if (image.empty()) {
        cerr << "Error: cannot read " << argv[1] << endl;

        return 1;
    }

    // separate the image in 3 places (B, G, R)
    vector<Mat> channels;
    split(image, channels);


    Mat hist_bgr[3];
    Mat cdf_bgr[3];

    // establish the number of bins
    int hist_size = 256;

    // ranges for B, G, R
    float range[]           = { 0, 256 } ;
    const float* hist_range = { range };

    // compute the histograms for each channel
    calcHist(&channels[0], 1, 0, Mat(), hist_bgr[0], 1, &hist_size, &hist_range, true, false);
    calcHist(&channels[1], 1, 0, Mat(), hist_bgr[1], 1, &hist_size, &hist_range, true, false);
    calcHist(&channels[2], 1, 0, Mat(), hist_bgr[2], 1, &hist_size, &hist_range, true, false);

    // compute average histogram from the 3 channels
    Mat average_hist = Mat::zeros(hist_bgr[0].size(), hist_bgr[0].type());
    Mat average_cdf;

    for (int row = 0; row < hist_bgr[0].rows; row++) {
        average_hist.at<float>(row, 0) = (hist_bgr[0].at<float>(row, 0) +
                                          hist_bgr[1].at<float>(row, 0) +
                                          hist_bgr[2].at<float>(row, 0)) / 3;
    }

    // compute CDF for each channel histogram
    cumsum(hist_bgr[0],  cdf_bgr[0]);
    cumsum(hist_bgr[1],  cdf_bgr[1]);
    cumsum(hist_bgr[2],  cdf_bgr[2]);
    cumsum(average_hist, average_cdf);


    // Histogram equalization
    // 
    Mat equalized;
    Mat channels_equal[3] = {
        Mat::zeros(channels[0].size(), channels[0].type()),
        Mat::zeros(channels[1].size(), channels[1].type()),
        Mat::zeros(channels[2].size(), channels[2].type()),
    };

    // apply histogram equalization for each channel separatly
    equalizeHist(channels[0], channels_equal[0]);
    equalizeHist(channels[1], channels_equal[1]);
    equalizeHist(channels[2], channels_equal[2]);

    // create final image from the separatly histogram-equalized channels
    merge(channels_equal, 3, equalized);


    // Histogram matching
    // 
    Mat matched;
    Mat channels_matched[3] = {
        Mat::zeros(channels[0].size(), channels[0].type()),
        Mat::zeros(channels[1].size(), channels[1].type()),
        Mat::zeros(channels[2].size(), channels[2].type()),
    };

    // apply histogram matching with the average of all CDFs for all channels
    // separatly
    histogram_matching(channels[0], average_cdf, channels_matched[0]);
    histogram_matching(channels[1], average_cdf, channels_matched[1]);
    histogram_matching(channels[2], average_cdf, channels_matched[2]);

    // create final image from the sepratly matched channels
    merge(channels_matched, 3, matched);


    // canvas for histograms and CDFs
    int hist_width  = 512;
    int hist_height = 400;

    Mat canvas_hist(hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));
    Mat canvas_cdf (hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));

    draw_histogram(hist_bgr[0],  canvas_hist, {255,  0,    0});
    draw_histogram(hist_bgr[1],  canvas_hist, {  0, 255,   0});
    draw_histogram(hist_bgr[2],  canvas_hist, {  0,   0, 255});
    // draw_histogram(average_hist, canvas_hist, {  0, 255, 255});

    draw_histogram(cdf_bgr[0],  canvas_cdf, {255,  0,    0});
    draw_histogram(cdf_bgr[1],  canvas_cdf, {  0, 255,   0});
    draw_histogram(cdf_bgr[2],  canvas_cdf, {  0,   0, 255});
    // draw_histogram(average_cdf, canvas_cdf, {  0, 255, 255});

    // display
    namedWindow("Original", CV_WINDOW_AUTOSIZE);
    imshow("Original", image);

    namedWindow("Histograms", CV_WINDOW_AUTOSIZE);
    imshow("Histograms", canvas_hist);

    namedWindow("CDFs", CV_WINDOW_AUTOSIZE);
    imshow("CDFs", canvas_cdf);

    namedWindow("Histogram Equalization", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Equalization", equalized);

    namedWindow("Histogram Matching", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Matching", matched);

    waitKey(0);

    return 0;
}