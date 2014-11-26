#include <iostream>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace std;
using namespace cv;

// Image containers for background and overlay image
Mat background;
Mat overlay;

// To calculate a homography, we need 4 corresponding points:
// the "original" points and the points after the transformation.

// 4 corner points(x,y) of the overlay image ("original" points before
// transformation)
vector<Point2f> corners;

// 4 points clicked by the user (mouse left click) in the background image
// (points after transformation)
vector<Point2f> clicks;


void merge_images(const Mat warped)
{
    // Create a masked version of the background where
    // the areas for the wapred image are black
    Mat gray;
    Mat mask;
    Mat masked_background;

    // Convert second image to gray-scale
    cvtColor(warped, gray, CV_BGR2GRAY);
    // Create binary image from gray-scale image
    threshold(gray, gray, 0, 255, CV_THRESH_BINARY);
    // Create mask
    bitwise_not(gray, mask);
    background.copyTo(masked_background, mask);

    // Render final image by simply add the 2 matrices
    imshow("Homography", masked_background + warped);
}


void on_mouse(int event, int x, int y, int flags, void* userdata)
{
    if (event != EVENT_LBUTTONDOWN) {
        return;
    }

    clicks.push_back(Point2f(float(x), float(y)));

    if (clicks.size() == 4) {
        cout << "Calculating homography ..." << endl;

        // Calculate homography between corners and clicks
        Mat homography = findHomography(corners, clicks, 0);

        // Create warped overlay image
        Mat warped;
        warpPerspective(overlay, warped, homography, background.size());

        // Merge background and warped image
        merge_images(warped);
    } else if (clicks.size() == 5) {
        cout << "Reset clicks" << endl;

        clicks.clear();
        imshow("Homography", background);
    }
}


int main(int argc, char const *argv[])
{
    // Argument parsing
    if (argc != 3) {
        cout << "Usage: ./homography <background> <overlay>" << endl;

        return 1;
    }

    // Load images
    background = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    overlay    = imread(argv[2], CV_LOAD_IMAGE_COLOR);

    // Check if files were found meaning the images are not empty
    if (background.empty()) {
        cerr << "Cannot read background image " << argv[1] << endl;

        return 1;
    }

    if (overlay.empty()) {
        cerr << "Cannot read overlay image " << argv[2] << endl;

         return 1;
    }

    // Load the 4 corner points of the overlay image
    corners.push_back(Point2f(float(0),            float(0)));
    corners.push_back(Point2f(float(0),            float(overlay.rows)));
    corners.push_back(Point2f(float(overlay.cols), float(overlay.rows)));
    corners.push_back(Point2f(float(overlay.cols), float(0)));

    // Display window
    namedWindow("Homography", WINDOW_AUTOSIZE);
    imshow("Homography", background);

    setMouseCallback("Homography", on_mouse, NULL);

    // Wait for a key stroke indefinitly
    waitKey(0);

    return 0;
}