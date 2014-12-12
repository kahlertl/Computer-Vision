#include "histogram.h"


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


void draw_histogram(const Mat &hist, Mat &canvas, Scalar color)
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


void histogram_matching(Mat &image, Mat &pattern_cdf, Mat &match)
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

    calcHist(&image, 1, 0, Mat(), image_hist, 1, &hist_size, &hist_range, true, false);

    cumsum(image_hist,   image_cdf);

    // create look up table
    Mat lut = Mat::zeros(image_hist.size(), image_hist.type());

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