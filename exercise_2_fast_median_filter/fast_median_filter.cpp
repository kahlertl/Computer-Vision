/**
 * Fast Median Filter
 * 
 * @author: Lucas Kahlert <lucas.kahlert@tu-dresden.de>
 */
#include <iostream> // std::cout
#include "opencv2/highgui/highgui.hpp" // cv:imread, cv::imshow, cv::waitKey

using namespace std;
using namespace cv;

// input image
Mat image;

int radius      = 3;
int window_size = 2 * radius + 1;
int window_area = window_size * window_size;

int histogram[256];

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

void huang_median()
{
    int median = 0;

    // for (int row = 0; row < window_size; row++) {
    //     for (int col = 0; col < window_size; col++) {
    //         histogram[image.at<uchar>(row, col)]++;
    //     }
    // }

    // median = histo_median(histogram);

    Mat filtered_image = Mat::zeros(image.size(), image.type());


    for (int row = radius; row < image.rows - radius; row++) {
        // init histogram

        // zero the histogram
        memset(histogram, 0, sizeof(histogram));

        for (int window_row = row - radius; window_row < row + radius; window_row++) {
            for (int col = 0; col < window_size; col++) {
                histogram[image.at<uchar>(window_row, col)]++;
            }
        }

        filtered_image.at<uchar>(row,radius) = histo_median(histogram);
        // continue;

        for (int col = radius + 1; col < image.cols - radius; col++) {
            // remove left column
            // cout << "Remove left column " << col - radius - 1 << endl;

            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col - radius - 1)]--;
            }

            // add right column
            // cout << "Add right column " << col + radius << endl;

            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col + radius)]++;
            }

            filtered_image.at<uchar>(row,col) = histo_median(histogram);
        }

        // break;

    }

    // while (true) {
    //     // move right
    //     for (int col = radius; col < image.cols - radius; col++) {
    //         // cout << row << " " << col << endl;

    //         // remove left column from last step
    //         for (int i = 0; i < window_size; i++) {
    //             histogram[image.at<uchar>(row - radius + i, col - radius - 1)]--;
    //         }

    //         // add right column
    //         for (int i = 0; i < window_size; i++) {
    //             histogram[image.at<uchar>(row - radius + i, col + radius)]++;
    //         }

    //         image.at<uchar>(row,col) = histo_median(histogram);
    //     }

    //     row++;

    //     if (row > image.rows - radius) {
    //         // cout << "Break at row " << row << endl;

    //         break;
    //     }

    //     // move down
    //     // 
    //     // remove top row
    //     for (int col = image.cols - (2 * radius + 1); col < image.cols; col++) {
    //         histogram[image.at<uchar>(row - radius - 1, col)]--;
    //     }

    //     // add botton row
    //     for (int col = image.cols - (2 * radius + 1); col < image.cols; col++) {
    //         histogram[image.at<uchar>(row + radius, col)]--;
    //     }


    //     // move left
    //     for (int col = image.cols - radius; col < radius; col--) {
    //         cout << row << " " << col << endl;

    //         // remove right column
    //         // add right column
    //         for (int i = 0; i < window_size; i++) {
    //             histogram[image.at<uchar>(row - radius + i, col + radius)]--;
    //         }

    //         // add left column
    //         for (int i = 0; i < window_size; i++) {
    //             histogram[image.at<uchar>(row - radius + i, col - radius - 1)]++;
    //         }

    //         image.at<uchar>(row,col) = histo_median(histogram);
    //     }

    //     row++;

    //     if (row > image.rows - radius) {
    //         cout << "Break at row " << row << endl;

    //         break;
    //     }

    //     // move down
    //     // 
    //     // remove top row
    //     for (int col = 2 * radius + 1; col >= 0; col--) {
    //         histogram[image.at<uchar>(row - radius - 1, col)]--;
    //     }

    //     // add botton row
    //     for (int col = 2 * radius + 1; col >= 0; col--) {
    //         histogram[image.at<uchar>(row + radius, col)]--;
    //     }
    // }

    imshow("Median filter", filtered_image);
}

/**
 * 
 */
void median_filter(int, void*)
{
    // // round off margin by integer division
    // int margin = radius / 2;

    // Mat filtered_image = Mat::zeros(image.rows - margin, image.cols - margin, image.type());

    

    // imshow("Median Filter", image);
    // imshow("Median Filter", filtered_image);

    huang_median();
}

int main(int argc, char const *argv[])
{
    // argument parsing
    if (argc != 2) {
        cout << "Usage: ./fast_median_filter <image>" << endl;

        return 1;
    }

    // image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    image = imread("fruits.jpg", CV_LOAD_IMAGE_GRAYSCALE);

    if (image.empty()) {
        cerr << "Cannot read image " << argv[1] << endl;

        return 1;
    }

    // zero the historgram
    memset(histogram, 0, sizeof(histogram));

    // namedWindow("Median Filter", WINDOW_AUTOSIZE);

    // create trackbar for filter size
    // createTrackbar("Filter size in px", "Median Filter", &radius, 255, median_filter);

    // apply filter
    // median_filter(0, NULL);
    huang_median();

    // wait for a key stroke indefinitly
    waitKey(0);

    return 0;
}