#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

Mat image;
Mat sharpend;

// scaling factor for the substraction of the smoothed image
int alpha = 10;

void on_trackbar(int, void*)
{
    // scaling factor
    float scale = alpha / 10.;

    // Unsharp masking kernel
    // 
    // constructed from the formula:
    // 
    //     sharpened = image + scale * (image - blur * image) = image * kernel
    // 
    //              [ 0 0 0 ]           ( [ 0 0 0 ]   [ 1/16 1/8 1/16 ] )   
    //     kernel = [ 0 1 0 ] + scale * ( [ 0 1 0 ] - [  1/8 1/4  1/8 ] )
    //              [ 0 0 0 ]           ( [ 0 0 0 ]   [ 1/16 1/8 1/16 ] )
    //
    Mat kernel = (Mat_<float>(3,3) << -1./16 * scale,            -1./8 * scale, -1./16 * scale,
                                       -1./8 * scale, 1 + scale - 1./4 * scale,  -1./8 * scale,
                                      -1./16 * scale,            -1./8 * scale, -1./16 * scale );

    // compute convolution
    filter2D(image, sharpend, -1, kernel);

    // display result
    imshow("Unsharp masking", sharpend);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        cout << "Usage: ./unsharp image" << endl;

        return 1;
    }

    // load image
    image = imread( argv[1], CV_LOAD_IMAGE_COLOR);

    if (image.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    // display
    namedWindow("Original image", CV_WINDOW_AUTOSIZE);
    imshow("Original image", image);

    namedWindow("Unsharp masking", CV_WINDOW_AUTOSIZE);
    createTrackbar("alpha", "Unsharp masking", &alpha, 100, on_trackbar);

    // initial rendering
    on_trackbar(0, 0);

    waitKey(0);

    return 0;
}