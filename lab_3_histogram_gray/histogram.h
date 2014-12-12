#ifndef HISTOGRAM_S
#define HISTOGRAM_S

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/**
 * Cumulative distribution function of a histogram.
 */
void cumsum(const Mat &hist, Mat &cdf_matrix);

/**
 * Draw the histograms for B, G and R
 */
void draw_histogram(const Mat &hist, Mat &canvas, Scalar color = {255, 255, 255});

/**
 * Perform histogram matching on an image using the CDF of the pattern
 * image. The result will be stored in the third matrix. * 
 */
void histogram_matching(Mat &image, Mat &pattern, Mat &match);

#endif