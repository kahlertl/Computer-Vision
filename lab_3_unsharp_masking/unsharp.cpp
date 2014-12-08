#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

Mat src;
Mat dst;

// scaling factor for the substraction of the smoothed image
int alpha = 25;

void on_trackbar(int, void*)
{
    // blure image
    GaussianBlur(src, dst, Size(0, 0), 3);
    // subtract the smoothed version from the source image (in a weighted way,
    // so th values of a constant area remain constant)
    addWeighted(src, 1. + (alpha / 100.), dst, -(alpha / 100.), 0, dst);

    imshow("Unsharp masking", dst);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        cout << "Usage: ./unsharp image" << endl;

        return 1;
    }

    // load image
    src = imread( argv[1], CV_LOAD_IMAGE_COLOR);

    if (src.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    // display
    namedWindow("Original image", CV_WINDOW_AUTOSIZE);
    imshow("Original image", src);

    namedWindow("Unsharp masking", CV_WINDOW_AUTOSIZE);
    createTrackbar("alpha", "Unsharp masking", &alpha, 100, on_trackbar);

    // initial rendering
    on_trackbar(0, 0);

    waitKey(0);

    return 0;
}