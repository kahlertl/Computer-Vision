#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

static void invert(Mat& image)
{
    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            image.at<float>(row, col) = 1.0 - image.at<float>(row, col);
        }
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        cerr << "Usage: ./magnitude <image>" << endl;
    }

    const string filename = argv[1];
    Mat image = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);

    if (image.empty()) {
        return -1;
    }

    // expand image to optimal size for DFT
    Mat padded;

    // performance of DFT depends on image size. It tends to be the fastest
    // for images sizes that are multiple of 2, 3 and 5
    // (see FFT - Fast Fourier Transformation)
    int m = getOptimalDFTSize(image.rows);
    int n = getOptimalDFTSize(image.cols);

    copyMakeBorder(image, padded,
        0,              // top
        m - image.rows, // bottom
        0,              // left
        n - image.cols, // right
        BORDER_CONSTANT,
        Scalar::all(0)  // fill border black
    );

    // Adds a complex channel to the current image.
    // All complex parts are zero for now
    Mat planes[] = {
        Mat_<float>(padded),
        Mat::zeros(padded.size(), CV_32F),
    };
    Mat complex_image;
    merge(planes, 2, complex_image);

    dft(complex_image, complex_image);

    // planes[0] = Re(complex_image)
    // planes[1] = Im(complex_image)
    split(complex_image, planes);

    // transform the real and complex values to magnitude
    // 
    //     M(I) = sqrt(Re(I)^2 + Im((I)^2)
    //     
    Mat magnitude_image;
    magnitude(planes[0], planes[1], magnitude_image);

    // switch to logarithmic scale
    magnitude_image += Scalar::all(1);
    log(magnitude_image, magnitude_image);

    // cut off the padded values, if it has an odd number of rows or cols
    magnitude_image = magnitude_image(Rect(0, 0,
                                      magnitude_image.cols & -2,
                                      magnitude_image.rows & -2));

    // Rearrange the quadrants of the fourier image, so that the origin is at the
    // image center

    int center_x = magnitude_image.cols / 2;
    int center_y = magnitude_image.rows / 2;

    // Create a ROI (region of image)
    Mat q0(magnitude_image, Rect(0, 0, center_x, center_y));        // top left
    Mat q1(magnitude_image, Rect(center_x, 0, center_x, center_y)); // top right
    Mat q2(magnitude_image, Rect(0, center_y, center_x, center_y)); // bottom left
    Mat q3(magnitude_image, Rect(center_x, center_y, center_x, center_y)); // bottom right

    // swap quadrants
    // 
    // top left with bottom right
    Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    // top right with bottom left
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    // Transform the matrix with float values [0,1] into a viewable grayscale
    // image.
    normalize(magnitude_image, magnitude_image, 0, 1, CV_MINMAX);

    // cout << magnitude_image << endl;

    invert(magnitude_image);

    imshow("Input", image);
    imshow("spectrum magnitude", magnitude_image);

    // wait for ESC key
    cout << "Press ESC to exit ..." << endl;
    while (true) {
        if ((uchar)waitKey(0) == 27) {
            break;
        }
    }

    return 0;
}