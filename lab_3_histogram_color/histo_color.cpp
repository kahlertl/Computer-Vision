#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

#include "histogram.h"

using namespace std;
using namespace cv;

/**
 * Compute cumulative distribution function
 */
void cumsum(Mat& hist, Mat& cdf)
{
    cdf = Mat::zeros(hist.size(), hist.type());

    float sum = 0;
    for (int row = 0; row < hist.rows; row++) {
        sum += hist.at<float>(row, 0);
        cdf.at<float>(row, 0) = sum;
    }
}

void histogram_equalization(const vector<Mat> &channels, const Mat (&hist_bgr)[3])
{
    // compute average histogram from the 3 channels
    Mat hist_average = Mat::zeros(hist_bgr[0].size(), hist_bgr[0].type());

    for (int row = 0; row < hist_bgr[0].rows; row++) {
        hist_average.at<float>(row, 0) = (hist_bgr[0].at<float>(row, 0) +
                                          hist_bgr[1].at<float>(row, 0) +
                                          hist_bgr[2].at<float>(row, 0)) / 3;
        // cout << hist_bgr[0].at<float>(row, 0) << " "
        //      << hist_bgr[1].at<float>(row, 0) << " "
        //      << hist_bgr[2].at<float>(row, 0) << " -> ";
        // cout << ((hist_bgr[0].at<float>(row, 0) +
        //           hist_bgr[1].at<float>(row, 0) +
        //           hist_bgr[2].at<float>(row, 0)) / 3) << endl;
    }

    cout << hist_average << endl;
}

int main( int argc, char** argv )
{
    Mat src;

    if (argc != 2) {
        cout << "Usage: ./histo_color image" << endl;

        return 1;
    }

    // load image
    src = imread(argv[1], 1);

    if (!src.data) {
        cerr << "Error: cannot read " << argv[1] << endl;

        return 1;
    }

    // separate the image in 3 places (B, G, R)
    vector<Mat> channels;
    split(src,  channels);

    // establish the number of bins
    int hist_size = 256;

    // ranges for B, G, R
    float range[]           = { 0, 256 } ;
    const float* hist_range = { range };

    bool uniform = true;
    bool accumulate = false;

    Mat hist_bgr[3];
    Mat cdf_bgr[3];

    // compute the histograms:
    calcHist(&channels[0], 1, 0, Mat(), hist_bgr[0], 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&channels[1], 1, 0, Mat(), hist_bgr[1], 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&channels[2], 1, 0, Mat(), hist_bgr[2], 1, &hist_size, &hist_range, uniform, accumulate);

    // compute CDFs of the histograms
    cumsum(hist_bgr[0], cdf_bgr[0]);
    cumsum(hist_bgr[1], cdf_bgr[1]);
    cumsum(hist_bgr[2], cdf_bgr[2]);

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
    Mat image_channel_equal;

    merge(channels_equal, 3, image_channel_equal);

    // histogram_equalization(channels, hist_bgr);

    // canvas for histograms and CDFs
    int hist_width  = 512;
    int hist_height = 400;

    Mat canvas_hist(hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));
    Mat canvas_cdf (hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));

    draw_histogram(hist_bgr[0], canvas_hist, {255,  0,    0});
    draw_histogram(hist_bgr[1], canvas_hist, {  0, 255,   0});
    draw_histogram(hist_bgr[2], canvas_hist, {  0,   0, 255});

    draw_histogram(cdf_bgr[0], canvas_cdf, {255,  0,    0});
    draw_histogram(cdf_bgr[1], canvas_cdf, {  0, 255,   0});
    draw_histogram(cdf_bgr[2], canvas_cdf, {  0,   0, 255});


    // display
    namedWindow("Histograms", CV_WINDOW_AUTOSIZE);
    imshow("Histograms", canvas_hist);

    namedWindow("CDFs", CV_WINDOW_AUTOSIZE);
    imshow("CDFs", canvas_cdf);

    namedWindow("Histogram Equalization (for each channel)", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Equalization (for each channel)", image_channel_equal);

    waitKey(0);

    return 0;
}