#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;


void cumsum(const Mat &hist, Mat &cdf_matrix)
{
    // initialize CDF matrix
    if (cdf_matrix.empty()) {
        cdf_matrix = Mat::zeros(hist.size(), hist.type());
    }

    float sum = 0;

    for (int row = 0; row < hist.rows; row++) {
        sum += hist.at<float>(row, 0);
        cdf_matrix.at<float>(row, 0) = sum;
    }
}


/**
 * Draw the histograms for B, G and R
 */
void draw_histogram(const Mat &hist, Mat &canvas, Scalar color = {255, 255, 255})
{
    const int hist_width  = canvas.cols;
    const int hist_height = canvas.rows; 
    const int bin_width   = cvRound((double) hist_width / hist.rows);
    
    // normalize the result to [ 0, canvas.rows ]
    Mat normalized;
    normalize(hist, normalized, 0, canvas.rows - 10, NORM_MINMAX, -1, Mat());

    for (int i = 1; i < normalized.rows; i++) {
        line(
            canvas,
            Point(bin_width * (i - 1), hist_height - cvRound(normalized.at<float>(i - 1))),
            Point(bin_width * (i),     hist_height - cvRound(normalized.at<float>(i)) ),
            color,
            2, 8, 0
        );
    }
}


void histogram_matching(Mat &image, Mat &pattern, Mat &match)
{
    // initialize matched image matrix if empty
    if (match.empty()) {
        match = Mat::zeros(image.size(), image.type());
    }
    
    // number of bins
    int hist_size = 256;

    // ranges for B,G,R
    const float  range[]    = { 0, 256 } ;
    const float* hist_range = { range };

    // computer CDF if the image that should be matched and  of the pattern
    // image with the nice histogram
    Mat image_hist;
    Mat image_cdf;
    Mat pattern_hist;
    Mat pattern_cdf;

    calcHist(&image,   1, 0, Mat(), image_hist,   1, &hist_size, &hist_range, true, false);
    calcHist(&pattern, 1, 0, Mat(), pattern_hist, 1, &hist_size, &hist_range, true, false);

    cumsum(image_hist,   image_cdf);
    cumsum(pattern_hist, pattern_cdf);

    // create look up table
    Mat lut = Mat::zeros(pattern_hist.size(), pattern_hist.type());

    for (int i = 0; i < hist_size; i++) {
        float frequency = image_cdf.at<float>(i);
        int color = 0;

        // search for the first CDF sum greater than the frequency of the color
        while (color < hist_size) {
            if (pattern_cdf.at<float>(color) >= frequency) {
                break;
            }
            color++;
        }

        lut.at<int>(i) = (color < hist_size) ? color : hist_size - 1;
    }

    // Mat match = Mat::zeros(image.size(), image.type());

    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            image.at<uchar>(row,col) = lut.at<int>(image.at<uchar>(row,col));
        }
    }
}


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