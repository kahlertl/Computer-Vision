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

    Mat hists_bgr[3];

    // compute the histograms:
    calcHist(&bgr_planes[0], 1, 0, Mat(), hists_bgr[0], 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, Mat(), hists_bgr[1], 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, Mat(), hists_bgr[2], 1, &hist_size, &hist_range, uniform, accumulate);

    Mat cdf_b;
    Mat cdf_g;
    Mat cdf_r;

    // compute CDFs of the histograms
    cumsum(hists_bgr[0], cdf_b);
    cumsum(hists_bgr[1], cdf_g);
    cumsum(hists_bgr[2], cdf_r);

    Mat channels_equal[3] = {
        Mat::zeros(bgr_planes[0].size(), bgr_planes[0].type()),
        Mat::zeros(bgr_planes[1].size(), bgr_planes[1].type()),
        Mat::zeros(bgr_planes[2].size(), bgr_planes[2].type()),
    };

    // apply histogram equalization for each channel separatly
    equalizeHist(bgr_planes[0], channels_equal[0]);
    equalizeHist(bgr_planes[1], channels_equal[1]);
    equalizeHist(bgr_planes[2], channels_equal[2]);

    // create final image from the separatly histogram-equalized channels
    Mat image_channel_equal;

    merge(channels_equal, 3, image_channel_equal);

    histogram_equalization(bgr_planes, hists_bgr);

    // draw the histograms for B, G and R
    int hist_wdith = 512;
    int hist_height = 400;
    int bin_width  = cvRound((double) hist_wdith / hist_size );

    Mat hist_image(hist_height, hist_wdith, CV_8UC3, Scalar( 0,0,0) );
    Mat cdf_image (hist_height, hist_wdith, CV_8UC3, Scalar( 0,0,0) );

    // normalize the histograms to [ 0, hist_image.rows ]
    normalize(hists_bgr[0], hists_bgr[0], 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(hists_bgr[1], hists_bgr[1], 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(hists_bgr[2], hists_bgr[2], 0, hist_image.rows - 10, NORM_MINMAX, -1, Mat());

    // normalize the CDFs to [ 0, cdf_image.rows ]
    normalize(cdf_b, cdf_b, 0, cdf_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(cdf_g, cdf_g, 0, cdf_image.rows - 10, NORM_MINMAX, -1, Mat());
    normalize(cdf_r, cdf_r, 0, cdf_image.rows - 10, NORM_MINMAX, -1, Mat());

    // draw for each channel
    for (int i = 1; i < hist_size; i++) {
        // histograms
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hists_bgr[0].at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(hists_bgr[0].at<float>(i))),
            Scalar(255, 0, 0),
            2, 8, 0
        );
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hists_bgr[1].at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(hists_bgr[1].at<float>(i))),
            Scalar(0, 255, 0),
            2, 8, 0
        );
        line(
            hist_image,
            Point(bin_width * (i - 1), hist_height - cvRound(hists_bgr[2].at<float>(i - 1))),
            Point(bin_width * i,       hist_height - cvRound(hists_bgr[2].at<float>(i))),
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

    namedWindow("Histogram Equalization (for each channel)", CV_WINDOW_AUTOSIZE );
    imshow("Histogram Equalization (for each channel)", image_channel_equal );

    waitKey(0);

    return 0;
}