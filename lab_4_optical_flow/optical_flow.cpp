#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>


using namespace std;
using namespace cv;


static void drawOpticalFlow(const Mat& flow, Mat& colorized)
{
    // extraxt x and y channels
    Mat xy[2]; // x,y 

    split(flow, xy);

    // calculate angle and magnitude for the HSV color wheel
    Mat magnitude;
    Mat angle;

    cartToPolar(xy[0], xy[1], magnitude, angle, true);

    // translate magnitude to range [0,1]
    double mag_max;
    minMaxLoc(magnitude, 0, &mag_max);

    magnitude.convertTo(
        magnitude,    // output matrix
        -1,           // type of the ouput matrix, if negative same type as input matrix
        1.0 / mag_max // scaling factor
    );

    // build hsv image (hue-saturation-value)
    Mat _hsv[3];
    Mat hsv;

    // create separte channels
    _hsv[0] = angle;                           // H (hue)              [0,360]
    _hsv[1] = magnitude;                       // S (saturation)       [0,1]
    _hsv[2] = Mat::ones(angle.size(), CV_32F); // V (brigthness value) [0,1]

    // merge the three components to a three channel HSV image
    merge(_hsv, 3, hsv);

    //convert to BGR and show
    cvtColor(hsv, colorized, cv::COLOR_HSV2BGR);
}


int main(int argc, char const *argv[])
{
    Mat previmage;
    Mat image;
    Mat flow;
    Mat colorized;

    if (argc != 3) {
        cout << "Usage: ./optical_flow previmage image" << endl;

        return 1;
    }

    // Load image
    previmage = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    image     = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (previmage.empty()) {
        cerr << "Error cannot read " << argv[1] << endl;

        return 1;
    }

    if (image.empty()) {
        cerr << "Error cannot read " << argv[2] << endl;

        return 1;
    }

    // calculate dense optical flow
    calcOpticalFlowFarneback(
        previmage,
        image,
        flow, // computed flow image that has the same size as prev and type CV_32FC2
        0.5,  // image scale: < 1 to build pyramids for each image. 0.5 means a
              // classical pyramid, where each next layer is twice smalller than the
              // previous one
        3,    // number of pyramid layers
        15,   // averaging windows size. larger values increase the algorithm robustness
              // to image noise and give more chances for fast motion detection, but
              // yields more blurred motion field
        3,    // number of iterations for each pyramid level
        5,    // size of the pixel neighborhood used to find the polynomial expansion
              // in each pixel
        1.2,  // standard deviation of the Gaussian used to smooth derivations
        0     // flags
    );

    // Colorize
    // cvtColor(previmage, colorized, COLOR_GRAY2BGR);

    // Draw flow map
    drawOpticalFlow(flow, colorized);

    // Display
    imshow("Optical flow", colorized);    

    cout << "Press ESC to exit ..." << endl;

    while (true) {
        if ((uchar) waitKey(0) == 27) {
            break;
        }
    }

    return 0;
}