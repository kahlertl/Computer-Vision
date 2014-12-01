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

int filter_size = 3;

/**
 * 
 */
void median_filter(int, void*)
{
    // round off margin by integer division
    int margin = filter_size / 2;

    Mat filtered_image = Mat::zeros(image.rows - margin, image.cols - margin, image.type());

    

    imshow("Median Filter", filtered_image);
}

int main(int argc, char const *argv[])
{
    // argument parsing
    if (argc != 2) {
        cout << "Usage: ./fast_median_filter <image>" << endl;

        return 1;
    }

    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

    if (image.empty()) {
        cerr << "Cannot read image " << argv[1] << endl;

        return 1;
    }

    namedWindow("Median Filter", WINDOW_AUTOSIZE);

    // create trackbar for filter size
    createTrackbar("Filter size in px", "Median Filter", &filter_size, 255, median_filter);

    // apply filter
    median_filter(0, NULL);

    // wait for a key stroke indefinitly
    waitKey(0);

    return 0;
}