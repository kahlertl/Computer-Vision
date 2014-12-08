#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

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


int main( int argc, char** argv )
{
    Mat src;
    Mat dst;

    if (argc != 2) {
        cout << "Usage: ./equalize_histo image" << endl;

        return 1;
    }

    // load image
    src = imread(argv[1], 1);

    if (!src.data) {
        cerr << "Error: cannot read " << argv[1] << endl;

        return 1;
    }

    // separate the image in 3 places (B, G, R)
    vector<Mat> bgr_planes;
    split(src,  bgr_planes);

    // establish the number of bins
    int hist_size = 256;

    // ranges for B, G, R
    float range[]           = { 0, 256 } ;
    const float* hist_range = { range };

    bool uniform = true;
    bool accumulate = false;

    Mat hist_b;
    Mat hist_g;
    Mat hist_r;

    // compute the histograms:
    calcHist(&bgr_planes[0], 1, 0, Mat(), hist_b, 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, Mat(), hist_g, 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, Mat(), hist_r, 1, &hist_size, &hist_range, uniform, accumulate);

    Mat cdf_b;
    Mat cdf_g;
    Mat cdf_r;

    // compute CDFs of the histograms
    cumsum(hist_b, cdf_b);
    cumsum(hist_g, cdf_g);
    cumsum(hist_r, cdf_r);

    // draw the histograms for B, G and R
    int hist_wdith = 512;
    int hist_height = 400;
    int bin_width  = cvRound((double) hist_wdith / hist_size );

    Mat hist_image(hist_height, hist_wdith, CV_8UC3, Scalar( 0,0,0) );
    Mat cdf_image (hist_height, hist_wdith, CV_8UC3, Scalar( 0,0,0) );

    // normalize the result to [ 0, hist_image.rows ]
    normalize(hist_b, hist_b, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(hist_g, hist_g, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(hist_r, hist_r, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());

    normalize(cdf_b, cdf_b, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(cdf_g, cdf_g, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(cdf_r, cdf_r, 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());


    // draw for each channel
    for (int i = 1; i < hist_size; i++) {
        // histograms
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hist_b.at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(hist_b.at<float>(i))),
            Scalar(255, 0, 0),
            2, 8, 0
        );
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hist_g.at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(hist_g.at<float>(i))),
            Scalar(0, 255, 0),
            2, 8, 0
        );
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hist_r.at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(hist_r.at<float>(i))),
            Scalar(0, 0, 255),
            2, 8, 0
        );
        // CDFs
        line(
            cdf_image,
            Point(bin_width * (i - 1), hist_height - cvRound(cdf_b.at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(cdf_b.at<float>(i))),
            Scalar(255, 0, 0),
            2, 8, 0
        );
        line(
            cdf_image,
            Point(bin_width * (i - 1), hist_height - cvRound(cdf_g.at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(cdf_g.at<float>(i))),
            Scalar(0, 255, 0),
            2, 8, 0
        );
        line(
            cdf_image,
            Point(bin_width * (i - 1), hist_height - cvRound(cdf_r.at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(cdf_r.at<float>(i))),
            Scalar(0, 0, 255),
            2, 8, 0
        );
    }

    // display
    namedWindow("Histograms", CV_WINDOW_AUTOSIZE );
    imshow("Histograms", hist_image );

    namedWindow("CDFs", CV_WINDOW_AUTOSIZE );
    imshow("CDFs", cdf_image );

    waitKey(0);

    return 0;
}