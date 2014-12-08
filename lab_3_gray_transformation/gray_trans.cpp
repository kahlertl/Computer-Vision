#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

Mat image;
Mat filtered_image;

// gamma
int g = 10;

void on_trackbar(int, void*)
{
    // normalizing constant to keep the value [0-255]
    float c = 255 / pow(255 + 1, g / 10.);

    // apply transformation function on each pixel in the image
    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            filtered_image.at<uchar>(row,col) = round(c * pow(image.at<uchar>(row,col) + 1, g / 10.));
        }
    }

    imshow("Gray transformation", filtered_image);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        cout << "Usage: ./gray_trans image" << endl;

        return 1;
    }

    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    
    if (image.empty()) {
        cerr << "Error: Cannot read '" << argv[1] << "'" << endl;

        return 1;
    }

    filtered_image = Mat::zeros(image.size(), image.type());

    // create interactive scene
    namedWindow("Gray transformation", 1);
    createTrackbar("gamma", "Gray transformation", &g, 127, on_trackbar);

    // initial rendering
    on_trackbar(0, NULL);

    // wait indefinitly on a key stroke
    waitKey(0);

    return 0;
}