#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

Mat image;
Mat patch;
Mat correlation;


void on_trackbar(int, void*)
{
    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            correlation.at<uchar>(row, col) = 
        }
    }


    imshow("Unsharp masking", patch);
}


int main(int argc, char const *argv[])
{
    if (argc != 3) {
        cout << "Usage: ./correlation patch image" << endl;

        return 1;
    }

    // load image
    patch = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    image = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (patch.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    if (image.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    correlation = Mat::zeros(image.size(), image.type());

    // display
    namedWindow("Template matching", CV_WINDOW_AUTOSIZE);
    imshow("Template matching", image);

    // initial rendering
    on_trackbar(0, 0);

    waitKey(0);

    return 0;
}