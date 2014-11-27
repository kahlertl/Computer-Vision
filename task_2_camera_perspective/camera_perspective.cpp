#include <iostream>
#include <math.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp" // projectPoints()



using namespace cv;
using namespace std;

// trackbar values
int camera_distance         = 200;
int camera_horizontal_angle = 0;
int camera_vertical_angle   = 0;
int affine_main_angle       = 0;
int affine_secondary_angle  = 0;
int affine_scale_factor     = 100;


//
int offset_x = 0;
int offset_y = 0;


// 
vector<Point3d> object_points;
vector<Point3d> axis_points_3d;

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



inline float radian(const float degree)
{
    return degree * M_PI / 180;
}


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

inline Mat RotX(const int angle)
{
    float theta = radian(angle);

    return (Mat_<double>(4,4) << 1,          0,           0, 0,
                                 0, cos(theta), -sin(theta), 0,
                                 0, sin(theta),  cos(theta), 0,
                                 0,          0,           0, 1); 
}

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
    rotation = RotX(camera_vertical_angle) * RotZ(camera_horizontal_angle);

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
    D.at<double>(0,0) = affine_scale_factor / 100.0;
    D.at<double>(1,1) = 1;

    // compose rotation and scaling matrices
    Mat A(2,2,CV_64F, cvScalar(0.0));
    A = Rot(affine_main_angle) * (
            Rot(-affine_secondary_angle) *
            D *
            Rot(affine_secondary_angle)
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


    // Create square dimensions
    // TODO
    resize(image, image, Size_<int>(512, 512));


    object_points = vector<Point3d>(image.rows * image.cols);

    // offset_x = image.rows / 2;
    // offset_y = image.cols / 2;
    int i = 0;

    for (int row = 0; row < image.rows; row++) {
        for (int col = 0; col < image.cols; col++) {
            Point3d point = Point3d(
                row - offset_x, // x
                col - offset_y, // y
                0               // z
            );

            object_points[i] = point;

            // cout << point << endl;
            i++;
        }
    }

    // for (int i = 0; i < 512; i++) {
    //     axis_points_3d.push_back(Point3d(0,0,i));
    //     axis_points_3d.push_back(Point3d(0,i,0));
    //     axis_points_3d.push_back(Point3d(i,0,0));
    // }

    
    // imshow("camera perspective", image);

    // trans_matrix = (Mat_<double>(2,3) << 1, 0, -image.rows / 2,
    //                                      0, 1, -image.cols / 2);

    // image for performing affine transformation
    affine_image = Mat::zeros(image.rows, image.cols, image.type());

    // create a window
    namedWindow("camera perspective", 1);

    // create trackbars for the camera position
    // TODO: reasonable range
    createTrackbar("distance", "camera perspective", &camera_distance, 1000, render);
    createTrackbar("horizontal", "camera perspective", &camera_horizontal_angle, 360, render);
    createTrackbar("vertical", "camera perspective", &camera_vertical_angle, 360, render);

    // create trackbars for affine transformation
    // TODO: reasonable range
    createTrackbar("main rotation angle", "camera perspective", &affine_main_angle, 360, render);
    createTrackbar("sec. rotation angle", "camera perspective", &affine_secondary_angle, 180, render);
    createTrackbar("scale factor in %", "camera perspective", &affine_scale_factor, 200, render);

    render(0, NULL);

    // wait for a key stroke; the same function arranges events processing
    waitKey(0);

    return 0;
}
