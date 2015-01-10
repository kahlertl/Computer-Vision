#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

Mat image;
Mat disparity;
Mat canvas;

int max_disp = 0;

int tracker_alpha = 210;
int tracker_beta  = 120;
int tracker_zoom  = 85;
int tracker_dist  = 40;


/**
 * Conversion from degree into radian
 */
inline float radian(const float degree)
{
    return degree * M_PI / 180;
}


/**
 * 3D rotation matrix around x-axis
 */
inline Mat RotX(const float angle)
{
    float alpha = radian(angle);

    return (Mat_<double>(3,3) << 1,          0,           0,
                                 0, cos(alpha), -sin(alpha),
                                 0, sin(alpha),  cos(alpha));
}


/**
 * 3D rotation matrix around y-axis
 */
inline Mat RotY(const float angle)
{
    float alpha = radian(angle);

    return (Mat_<double>(3,3) <<  cos(alpha), 0, sin(alpha),
                                           0, 1,          0,
                                 -sin(alpha), 0, cos(alpha));
}


/**
 * Callback function for all trackbars
 */
void render(int, void*)
{

    Mat rot_x = RotX((tracker_alpha - 180));
    Mat rot_y = RotY((tracker_beta  - 180));

    double zoom = tracker_zoom / 100.0;
    double dist = (50 - tracker_dist + 1);

    double mindisp;
    double maxdisp;

    minMaxLoc(disparity, &mindisp, &maxdisp);

    // reset canvas
    canvas.setTo(0);


    Mat depth_matrix = Mat(canvas.size(), DataType<double>::type);

    depth_matrix.setTo(INFINITY);

    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {

            if (disparity.at<uchar>(row, col) == 0) {
                continue;
            }

            double depth = maxdisp / disparity.at<uchar>(row, col) + dist;
            // double depth = 1;

            Mat point = (Mat_<double>(3,1) << col - image.cols / 2,
                                              row - image.rows / 2,
                                              depth);


            // zooming
            point.at<double>(0,0) *= zoom;
            point.at<double>(1,0) *= zoom;

            // rotating
            point = rot_x * rot_y * point;

            // revert centering
            // 
            // we do not revert the whole translation of the half of the image sizes, because there
            // are a few outliers that would move the point cloud in the top left corner
            point.at<double>(0,0) += image.cols / 1.5; // 2;
            point.at<double>(1,0) += image.rows / 1.5; // 2;

            
            const int x2d = point.at<double>(0,0) / depth * dist;
            const int y2d = point.at<double>(1,0) / depth * dist;

            // const int x2d = point.at<double>(0,0); // cols
            // const int y2d = point.at<double>(1,0); // rows

            if (x2d > 0 && x2d < canvas.cols && y2d > 0 && y2d < canvas.rows) {
                if (depth < depth_matrix.at<double>(y2d, x2d)) {
                    depth_matrix.at<double>(y2d, x2d) = depth;
                    canvas.at<Vec3b>(y2d, x2d)        = image.at<Vec3b>(row, col);
                }
            }
        }
        // cout << endl;
    }

    imshow("cloud", canvas);
}



int main(int argc, char const *argv[])
{
    // Argument parsing
    if (argc != 3) {
        cerr << "Usage: ./camera_perspective <image> <disparity map>" << endl;

        return 1;
    }

    image     = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    disparity = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

    if (image.empty()) {
        cerr << "Cannot read the image file." << endl;

        return 1;
    }

    if (disparity.empty()) {
        cerr << "Cannot read the disparity map files." << endl;

        return 1;
    }

    // normalize(disparity, disparity, 0, 16, NORM_MINMAX);

    canvas = Mat(image.size(), image.type());

    // canvas view
    namedWindow("cloud", WINDOW_AUTOSIZE);

    // create trackbars for the camera position
    createTrackbar("alpha",    "cloud", &tracker_alpha, 360, render);
    createTrackbar("beta",     "cloud", &tracker_beta,  360, render);
    createTrackbar("zoom",     "cloud", &tracker_zoom,  200, render);
    createTrackbar("distance", "cloud", &tracker_dist,  50, render);

    // initial rendering of the scene
    render(0, NULL);

    while (true) {
        // wait for a key stroke; the same function arranges events processing
        if ((uchar) waitKey(0) == 27) {
            break;
        }
    }

    return 0;
}