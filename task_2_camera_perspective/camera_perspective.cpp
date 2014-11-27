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
Mat image_warp;
Mat warp_matrix(2, 3, CV_64F, cvScalar(0.0));

// intrinsic camera parameter matrix
Mat camera_matrix = (Mat_<double>(3,3) << 100, 0, 512 / 2,
                                          0, 100, 512 / 2,
                                          0, 0, 1);



// void print_matrix(const Mat& matrix)
// {
//     for (int i = 0; i < matrix.rows; i++) {
//         for (int j = 0; j < matrix.cols; j++) {
//             cout << matrix.at<double>(i,j) << " ";
//         }
//         cout << endl;
//     }

//     cout << endl;
// }



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

inline Mat Rot_x(int theta)
{
    float rad = radian(theta);

    return (Mat_<double>(3,3) << 1, 0,        0,
                                 0, cos(rad), -sin(rad),
                                 0, sin(rad), cos(rad)); 
}

inline Mat Rot_z(int phi)
{
    float rad = radian(phi);

    return (Mat_<double>(3,3) << cos(rad), -sin(rad), 0,
                                 sin(rad), cos(rad) , 0,
                                 0,        0,         1);
}


void changeCameraPosition()
{
    float theta = radian(camera_vertical_angle);
    float phi   = radian(camera_horizontal_angle);

    Mat trans_matrix = (Mat_<double>(3,1) << camera_distance * sin(theta) * cos(phi),
                                             camera_distance * sin(theta) * sin(phi),
                                             camera_distance * cos(theta));


    vector<cv::Point2d> image_points;
    vector<cv::Point2d> axis_points_2d;

    // cout << image_points << endl;
    // cout << object_points << endl;

    projectPoints(
        object_points,
        Rot_z(camera_horizontal_angle) * Rot_x(camera_vertical_angle),
        trans_matrix,
        camera_matrix,
        noArray(), // do disortion
        image_points
    );

    projectPoints(
        axis_points_3d,
        Rot_z(camera_horizontal_angle) * Rot_x(camera_vertical_angle),
        trans_matrix,
        camera_matrix,
        noArray(), // do disortion
        axis_points_2d
    );

    // cout << image_points << endl;

    Mat camera_image = Mat::zeros(512, 512, image.type());

    Vec3b white = {255, 255, 255};
    Vec3b yellow = {255, 0, 255};
    Vec3b green = {0, 255, 0};

    for (int i = 0; i < image_points.size(); i++) {
        Point2d point = image_points[i];

        Point3d object_point = object_points[i];

        int row = point.x + offset_x;
        int col = point.y + offset_y;

        // cout << point << " -> " << row << ", " << col << endl;


        if (row >= 0 && row < image.rows) {
            if (col >= 0 && col < image.cols) {
                camera_image.at<Vec3b>(row, col) = image.at<Vec3b>(object_point.x, object_point.y);
            }
        }


    }

    // Axises
    for (int i = 0; i < axis_points_2d.size(); i++) {
        Point2d point = axis_points_2d[i];
        Point3d point_3d = axis_points_3d[i];

        // x-axis
        if (point_3d.z == 0 && point_3d.y == 0) {
            if (point.x >= 0 && point.x < 512 && point.y >= 0 && point.y < 512) {
                camera_image.at<Vec3b>(point.x, point.y) = white;
            }
        }
        // y-axis
        else if (point_3d.z == 0 && point_3d.x == 0) {
            if (point.x >= 0 && point.x < 512 && point.y >= 0 && point.y < 512) {
                camera_image.at<Vec3b>(point.x, point.y) = green;
            }
        }
        // z-axis
        else if (point_3d.x == 0 && point_3d.y == 0) {
            if (point.x >= 0 && point.x < 512 && point.y >= 0 && point.y < 512) {
                camera_image.at<Vec3b>(point.x, point.y) = yellow;
            }
        }
    }

    imshow("Camera", camera_image);

}


void affine_transformation() {
    // Calculate 2x3 warp matrix
    // Scaling matrix D
    Mat D(2,2,CV_64F, cvScalar(0.0));
    D.at<double>(0,0) = affine_scale_factor / 100.0;
    D.at<double>(1,1) = 1;

    Mat A(2,2,CV_64F, cvScalar(0.0));
    A = Rot(affine_main_angle) * (
            Rot(-1 * affine_secondary_angle) *
            D *
            Rot(affine_secondary_angle)
        );

    // 2x3 matrix (opencv knows last row)
    // translation is not needed therefore the third column stays zero
    warp_matrix.at<double>(0,0) = A.at<double>(0,0);
    warp_matrix.at<double>(0,1) = A.at<double>(0,1);
    warp_matrix.at<double>(1,0) = A.at<double>(1,0);
    warp_matrix.at<double>(1,1) = A.at<double>(1,1);

    // warp_matrix.at<double>(0,2) = image.rows / 2;
    // warp_matrix.at<double>(1,2) = image.cols / 2;

    cout << "Warp matrix:" << endl << warp_matrix << endl;

    // Apply transformation
    // TODO: decide on which image it should be applied (original or with camera perspective)
    warpAffine(image, image_warp, warp_matrix, image.size());

    // warpAffine(image_warp, image_warp, trans_matrix, image_warp.size());

    imshow("camera perspective", image_warp);
}

void transform(int, void*) {
    // affine_transformation();
    changeCameraPosition();
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

    for (int i = 0; i < 512; i++) {
        axis_points_3d.push_back(Point3d(0,0,i));
        axis_points_3d.push_back(Point3d(0,i,0));
        axis_points_3d.push_back(Point3d(i,0,0));
    }

    
    imshow("camera perspective", image);

    // trans_matrix = (Mat_<double>(2,3) << 1, 0, -image.rows / 2,
    //                                      0, 1, -image.cols / 2);

    // Image for performing transformation on
    image_warp = Mat::zeros(image.rows, image.cols, image.type());

    // Create a window
    namedWindow("camera perspective", 1);

    // Create trackbars for the camera position
    // TODO: reasonable range
    createTrackbar("distance", "camera perspective", &camera_distance, 1000, transform);
    createTrackbar("horizontal angle", "camera perspective", &camera_horizontal_angle, 360, transform);
    createTrackbar("vertical angle", "camera perspective", &camera_vertical_angle, 180, transform);

    // Create trackbars for affine transformation
    // TODO: reasonable range
    createTrackbar("main rotation angle", "camera perspective", &affine_main_angle, 360, transform);
    createTrackbar("sec. rotation angle", "camera perspective", &affine_secondary_angle, 180, transform);
    createTrackbar("scale factor in %", "camera perspective", &affine_scale_factor, 200, transform);

    // Wait for a key stroke; the same function arranges events processing
    waitKey(0);

    return 0;
}
