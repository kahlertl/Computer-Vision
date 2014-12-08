/**
 * Fast Threshold Median Filter
 *
 * @author: Lucas Kahlert <lucas.kahlert@tu-dresden.de>
 */
#include <iostream> // std::cout
#include <sstream>
#include <getopt.h> // getopt_long()
#include "opencv2/highgui/highgui.hpp" // cv:imread, cv::imshow, cv::waitKey

using namespace std;
using namespace cv;

// images
Mat image;
Mat filtered_image;

// filter parameters
int radius = 1;
int window_size;
int window_area;

// if the median is greater of equal this threshold the median
// is applied
int threshold = 0;

// histogram
int histogram[256];


/**
 * This function calculates the median of a single historgram
 * by summing all values from zero and stop if the sum reaches
 * the middle if the histogram width.
 *
 * The current index in the historgram is the median.
 */
static inline int histo_median(const int (&histogram)[256])
{
    int i = 0;

    for (int sum = 0; i < 256; i++) {
        sum += histogram[i];

        if (sum > window_area / 2) {
            break;
        }
    }

    return i;
}


static inline bool above_threshold(const int (&histogram)[256])
{
    for (int i = 0; i < threshold; i++) {
        if (histogram[i] > 0) {
            return false;
        }
    }

    return true;
}


void huang_median()
{
    // zero the histogram
    memset(histogram, 0, sizeof(histogram));

    // init histogram
    for (int row = 0; row < window_size; row++) {
        for (int col = 0; col < window_size; col++) {
            histogram[image.at<uchar>(row,col)]++;
        }
    }

    int row = radius;
    int median = 0;

    /*
     * We move in a snake like shape through the image. With that approach we
     * do not need to initialize the historgram for each row. 
     */
    while (true) {
        filtered_image.at<uchar>(row,radius) = (above_threshold(histogram))
                                               ? histo_median(histogram)
                                               : image.at<uchar>(row,radius);

        // move right
        for (int col = radius + 1; col < image.cols - radius; col++) {
            // remove left column
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col - radius - 1)]--;
            }

            // add right column
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col + radius)]++;
            }

            // calculate median for each channel
            filtered_image.at<uchar>(row,col) = (above_threshold(histogram))
                                                ? histo_median(histogram)
                                                : image.at<uchar>(row,col);
        }


        // right edge. move down
        row++;

        // check if we have reached bottom row
        if (row >= image.rows - radius) {
            break;
        }

        // remove top row
        for (int col = image.cols - window_size; col < image.cols; col++) {
            histogram[image.at<uchar>(row - radius - 1, col)]--;
        }

        // add bottom row
        for (int col = image.cols - window_size; col < image.cols; col++) {
            histogram[image.at<uchar>(row + radius, col)]++;
        }

        filtered_image.at<uchar>(row,image.cols - radius - 1) = (above_threshold(histogram))
                                                                ? histo_median(histogram)
                                                                : image.at<uchar>(row,image.cols - radius - 1);

        // move left
        for (int col = image.cols - radius - 2; col >= radius; col--) {
            // remove right column
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col + radius + 1)]--;
            }

            // add left column
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col - radius)]++;
            }

            filtered_image.at<uchar>(row,col) = (above_threshold(histogram))
                                                ? histo_median(histogram)
                                                : image.at<uchar>(row,col);
        }

        // left edge. move down
        row++;

        // check if we have reached bottom row
        if (row >= image.rows - radius) {
            break;
        }

        // remove top row
        for (int col = 0; col < window_size; col++) {
            histogram[image.at<uchar>(row - radius - 1, col)]--;
        }

        // add bottom row
        for (int col = 0; col < window_size; col++) {
            histogram[image.at<uchar>(row + radius, col)]++;
        }
    }
}


void on_trackbar(int, void*)
{
    window_size = 2 * radius + 1;
    window_area = window_size * window_size;

    filtered_image = Mat::zeros(image.size(), image.type());

    huang_median();

    imshow("Median filter", filtered_image);
}


int main(int argc, const char* argv[])
{
    if (argc != 2) {
        cout << "Usage: ./threshold_median_filter image" << endl;

        return 1;
    }

    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

    if (image.empty()) {
        cerr << "Error: cannot read " << argv[1] << endl;

        return 1;
    }

    // create interactive scene
    namedWindow("Median filter", 1);
    createTrackbar("radius", "Median filter",    &radius,    50,  on_trackbar);
    createTrackbar("threshold", "Median filter", &threshold, 255, on_trackbar);

    // initial rendering
    on_trackbar(0, NULL);

    // wait indefinitly on a key stroke
    waitKey(0);

    return 0;
}