#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>


using namespace std;
using namespace cv;


static void drawOptFlowMap(const Mat& flow, Mat& color_flow_map, int step, const Scalar& color)
{
    for (int y = 0; y < color_flow_map.rows; y += step) {
        for (int x = 0; x < color_flow_map.cols; x += step) {
            const Point2f& fxy = flow.at<Point2f>(y, x);

            line(color_flow_map, Point(x,y), Point(cvRound(x + fxy.x), cvRound(y + fxy.y)),
                 color);
            circle(color_flow_map, Point(x,y), 2, color, -1);
        }
    }
}


int main(int argc, char const *argv[])
{
    Mat previmage;
    Mat image;
    Mat flow;
    Mat color_flow;

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
    // cvtColor(previmage, color_flow, COLOR_GRAY2BGR);

    // Draw flow map
    // drawOptFlowMap(flow, color_flow, 16, Scalar(0, 255, 0));

    // Display
    // imshow("Optical flow", color_flow);    
    // imshow("Optical flow", flow);    
    cvWaitKey(0);

    return 0;
}