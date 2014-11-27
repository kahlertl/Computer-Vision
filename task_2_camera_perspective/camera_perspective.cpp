#include <iostream>
#include <math.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"

using namespace cv;
using namespace std;

// trackbar values
int camera_distance   = 200;
int camera_horizontal = 0;
int camera_vertical   = 0;

int affine_alpha   = 0;
int affine_beta    = 0;
int affine_scaling = 100;

vector<Point3d> object_points;

// images and matrixes
Mat image;
Mat affine_image;
Mat perspective_image;
Mat affine_matrix(2, 3, CV_64F, cvScalar(0.0));

// intrinsic camera parameter matrix
Mat camera_matrix = (Mat_<double>(3,4) << 100,   0, 512 / 2, 0,
                                            0, 100, 512 / 2, 0,
                                            0,   0,       1, 0);

// projection matrix from 2D -> 3D
Mat projection_3D = (Mat_<double>(4,3) << 1, 0, -512 / 2,
                                          0, 1, -512 / 2,
                                          0, 0,        0,
                                          0, 0,        1);


/**
 * Conversion from degree into radian
 */
inline float radian(const float degree)
{
    return degree * M_PI / 180;
}


/**
 * Simple 2D rotation matrix
 */
Mat Rot(int angle)
{
    Mat R(2,2,CV_64F, cvScalar(0.0));
    float rad = radian(angle);

    R.at<double>(0,0) = cos(rad);
    R.at<double>(0,1) = -1 * sin(rad);
    R.at<double>(1,0) = sin(rad);
    R.at<double>(1,1) = cos(rad);

    return R;
}

/**
 * 3D rotation matrix around x-axis in homogenous coordinates
 */
inline Mat RotX(const int angle)
{
    float theta = radian(angle);

    return (Mat_<double>(4,4) << 1,          0,           0, 0,
                                 0, cos(theta), -sin(theta), 0,
                                 0, sin(theta),  cos(theta), 0,
                                 0,          0,           0, 1); 
}

/**
 * 3D rotation matrix around z-axis in homogenous coordinates
 */
inline Mat RotZ(const int angle)
{
    float phi = radian(angle);

    return (Mat_<double>(4,4) << cos(phi), -sin(phi), 0, 0,
                                 sin(phi),  cos(phi), 0, 0,
                                        0,         0, 1, 0,
                                        0,         0, 0, 1);
}


void perspective_transformation()
{
    // rotation
    Mat rotation(3, 3, CV_64F, cvScalar(0.0));
    rotation = RotX(camera_vertical) * RotZ(camera_horizontal);

    // translation
    Mat translation = (Mat_<double>(4,4) << 1, 0, 0, 0,
                                            0, 1, 0, 0,
                                            0, 0, 1, camera_distance,
                                            0, 0, 0, 1);

    // compose transformation matrix
    Mat transformation = camera_matrix * (translation * (rotation * projection_3D));

    warpPerspective(affine_image, perspective_image, transformation, perspective_image.size());
    // warpPerspective(image, perspective_image, transformation, perspective_image.size());
    imshow("camera perspective", perspective_image);
}


void affine_transformation() {
    // calculate 2x3 warp matrix

    // scaling matrix
    Mat D(2,2,CV_64F, cvScalar(0.0));
    D.at<double>(0,0) = affine_scaling / 100.0;
    D.at<double>(1,1) = 1;

    // compose rotation and scaling matrices
    Mat A(2,2,CV_64F, cvScalar(0.0));
    A = Rot(affine_alpha) * (
            Rot(-affine_beta) *
            D *
            Rot(affine_beta)
        );

    // translation is not needed therefore the third column stays zero

    // affine_matrix.at<double>(0,2) = image.rows / 2;
    // affine_matrix.at<double>(1,2) = image.cols / 2;

    // compose finial transformation matrix
    affine_matrix.at<double>(0,0) = A.at<double>(0,0);
    affine_matrix.at<double>(0,1) = A.at<double>(0,1);
    affine_matrix.at<double>(1,0) = A.at<double>(1,0);
    affine_matrix.at<double>(1,1) = A.at<double>(1,1);

    // cout << "Warp matrix:" << endl << affine_matrix << endl;

    // apply transformation
    warpAffine(image, affine_image, affine_matrix, image.size());

    imshow("camera perspective", affine_image);
}


/**
 * Callback function for all trackbars
 */
void render(int, void*)
{
    affine_transformation();
    perspective_transformation();
}


int main( int argc, const char** argv )
{
    // Argument parsing
    if (argc != 2) {
        cerr << "Usage: ./camera_perspective <image>" << endl;

        return 1;
    }

    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);

    if (image.empty()) {
        cerr << "Cannot read the image files." << endl;

        return 1;
    }

    object_points = vector<Point3d>(image.rows * image.cols);

    // create xy-plane for the image
    for (int row = 0, i = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            Point3d point = Point3d(
                row - offset_x, // x
                col - offset_y, // y
                0               // z
            );
            object_points[i] = point;
            i++;
        }
    }

    // image for performing affine transformation
    affine_image = Mat::zeros(image.rows, image.cols, image.type());

    namedWindow("camera perspective", WINDOW_AUTOSIZE);

    // create trackbars for the camera position
    createTrackbar("distance",   "camera perspective", &camera_distance,   1000, render);
    createTrackbar("horizontal", "camera perspective", &camera_horizontal,  360, render);
    createTrackbar("vertical",   "camera perspective", &camera_vertical,    360, render);

    // create trackbars for affine transformation
    createTrackbar("affine ain rotation 1", "camera perspective", &affine_alpha,   360, render);
    createTrackbar("affine rotation 2",     "camera perspective", &affine_beta,    180, render);
    createTrackbar("affine scaling in %",   "camera perspective", &affine_scaling, 200, render);

    // initial rendering of the scene
    render(0, NULL);

    // wait for a key stroke; the same function arranges events processing
    waitKey(0);

    return 0;
}
