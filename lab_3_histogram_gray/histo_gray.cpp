#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

void cumsum(const Mat &hist, Mat &cdf_matrix)
{
    // reset CDF matrix
    cdf_matrix = Mat::zeros(hist.size(), hist.type());

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


int main(int argc, char** argv)
{
    Mat image;
    Mat equalized;

    if (argc != 2) {
        cout << "Usage: ./histo_match image" << endl;

        return 1;
    }

    // Load image
    image = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE);

    if( !image.data ) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    // Establish the number of bins
    int hist_size = 256;

    // Set the ranges (for B,G,R) )
    const float  range[]    = { 0, 256 } ;
    const float* hist_range = { range };

    const bool uniform    = true;
    const bool accumulate = false;

    Mat hist;

    // compute the histogram
    calcHist(&image, 1, 0, Mat(), hist, 1, &hist_size, &hist_range, uniform, accumulate);

    // compute cumulative distribution function
    Mat cdf;
    cout << "Reached" << endl;
    cumsum(hist, cdf);
        
    int hist_width  = 512;
    int hist_height = 400;

    Mat hist_image(hist_height, hist_width, CV_8UC3, Scalar( 0,0,0));

    draw_histogram(hist, hist_image, {  0,   0, 255});
    draw_histogram(cdf , hist_image, {255,   0,   0});

    // apply histogram equalization
    equalizeHist(image, equalized);

    // display
    namedWindow("Source", CV_WINDOW_AUTOSIZE);
    imshow("Source", image);

    namedWindow("Histogram", CV_WINDOW_AUTOSIZE);
    imshow("Histogram", hist_image);

    namedWindow("Histogram Equalization", CV_WINDOW_AUTOSIZE);
    imshow("Histogram Equalization", equalized);

    waitKey(0);

    return 0;
}