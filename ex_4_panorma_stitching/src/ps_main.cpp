// standard stuff
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>  // KeyPoint
#include <opencv2/nonfree/features2d.hpp>     // SurfFeatureDetector 

using namespace std;
using namespace cv;

// the header for all functions/macros of this project
#include "ps.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        cerr << "Usage: ps <left image> <right image>" << endl;
        return 0;
    }

    // read images
    Mat img_left = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    if (img_left.empty()) {
        cerr << "Can not read " << argv[1] << endl;
        return 1;
    }
    int height_left = img_left.rows;
    int width_left = img_left.cols;

    Mat img_right = imread(argv[2], CV_LOAD_IMAGE_COLOR);
    if (img_right.empty()) {
        cerr << "Can not read " << argv[2] << endl;
        return 1;
    }
    int height_right = img_right.rows;
    int width_right  = img_right.cols;

    // faust regel for window sizes :-)
    // + 0.5 for round up
    int wsize_sum   = (width_left + width_right + height_left + height_right) / 1300. + 0.5;
    int wsize_loc   = (width_left + width_right + height_left + height_right) / 500. + 0.5;
    int wsize_match = (width_left + width_right + height_left + height_right) / 90. + 0.5;

    if (wsize_sum < 1) wsize_sum = 1;
    if (wsize_loc < 2) wsize_loc = 2;
    if (wsize_match < 5) wsize_match = 5;
    if (wsize_match > 40) wsize_match = 40;

    cout << "Images loaded. wsize_sum=" << wsize_sum
         << ", wsize_loc="              << wsize_loc
         << ", wsize_match="            << wsize_match
         << endl;

//     // Harris detector
//     cerr << endl << "Start Harris detector ...";
//     vector<KEYPOINT> pointsl = harris(height_left, width_left, img_left.ptr(0), wsize_sum, wsize_loc, "L");
//     cerr << endl << pointsl.size() << " keypoints in the left image";
// #ifdef SAVE_ALL
//     save_keypoints_as_image(height_left, width_left, img_left.ptr(0), pointsl, "keypointsL.png");
// #endif

//     vector<KEYPOINT> pointsr = harris(height_right, width_right, img_right.ptr(0), wsize_sum, wsize_loc, "R");
//     cerr << endl << pointsr.size() << " keypoints in the right image";
// #ifdef SAVE_ALL
//     save_keypoints_as_image(height_right, width_right, img_right.ptr(0), pointsr, "keypointsR.png");
// #endif

//     // Matching
//     cerr << endl << "Start matching ...";
//     vector<MATCH> matches = matching(height_left, width_left, img_left.ptr(0), height_right, width_right, img_right.ptr(0), pointsl, pointsr, wsize_match);
//     cerr << endl << matches.size() << " matching pairs found";
// #ifdef SAVE_ALL
//     save_matches_as_image(height_left, width_left, img_left.ptr(0), height_right, width_right, img_right.ptr(0), matches, "matches.png");
// #endif

    // Feature / keypoint detection
    // 
    vector<KeyPoint> keypoints_left, keypoints_right;

    int minHessian = 600;
    SurfFeatureDetector detector(minHessian);

    cout << "Detect keypoints ..." << endl;

    detector.detect(img_left, keypoints_left);
    detector.detect(img_right, keypoints_right);

    // FAST(img_left,  keypoints_left,  50, true);
    // FAST(img_right, keypoints_right, 50, true);
    
    cout << "Non maximum suppression ..." << endl;

    // non-maximum-suppression
    // 
    suppressNonMax(img_left.cols,  img_left.rows,  keypoints_left,  width_left  * 0.02);
    suppressNonMax(img_right.cols, img_right.rows, keypoints_right, width_right * 0.02);

    #ifdef SAVE_ALL
        save_keypoints_as_image(img_left,  keypoints_left,  "keypointsL.png");
        save_keypoints_as_image(img_right, keypoints_right, "keypointsR.png");
    #endif


    // Calculate descriptors (alias feature vectors)
    // 
    SurfDescriptorExtractor extractor;
    Mat descriptors_left, descriptors_right;

    cout << "Compute feature descriptors ..." << endl;

    extractor.compute(img_left,  keypoints_left,  descriptors_left);
    extractor.compute(img_right, keypoints_right, descriptors_right);

    
    // Matching
    // 
    // FlannBasedMatcher matcher;
    BFMatcher matcher;
    vector<DMatch> matches;

    cout << "Matching ..." << endl;

    matcher.match(descriptors_left, descriptors_right, matches);

    #ifdef SAVE_ALL
        Mat img_matches;
        drawMatches(
            img_left,  keypoints_left,               // left image with its keypoints
            img_right, keypoints_right,              // right image with its keypoints
            matches,                                 // matches between the keypoints
            img_matches,                             // output image
            Scalar::all(-1),                         // color of matches
            Scalar::all(-1),                         // color of single points
            vector<char>(),                          // mask determining which matches are drawn. If empty
                                                     // all matches are drawn 
            DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS // Single keypoints will not be drawn
        );
        imwrite("matches.png", img_matches);
    #endif

    // homography -- OpenCV implementation
    // cout  << "Start standard RANSAC ..." << endl;
    // vector<Point2d> points_left;
    // vector<Point2d> points_right;
    // for (int i = 0; i < matches.size(); i++) {
    //     // points_left.push_back(Point2d(matches[i].xl, matches[i].yl));
    //     // points_right.push_back(Point2d(matches[i].xr, matches[i].yr));
    //     points_left.push_back(keypoints_left[matches[i].queryIdx].pt);
    //     points_right.push_back(keypoints_right[matches[i].trainIdx].pt);
    // }
    // Mat H = findHomography(points_left, points_right, CV_RANSAC);
    // Mat Hi = H.inv();
    // cout << " done";

    // // render the output
    // Mat imglt = img_right.clone();
    // warpPerspective(img_left, imglt, H, imglt.size(), INTER_LINEAR);
    // imglt = imglt * 0.5 + img_right * 0.5;
    // imwrite("warpedL.png", imglt);
    // Mat imgrt = img_left.clone();
    // warpPerspective(img_right, imgrt, Hi, imgrt.size(), INTER_LINEAR);
    // imgrt = imgrt * 0.5 + img_left * 0.5;
    // imwrite("warpedR.png", imgrt);
    // cout  << "Simple rendering done" << endl;

    // find two homographies
    cout  << "Start another RANSAC ..." << endl;
    Mat Hl, Hr;
    my_homographies(keypoints_left, keypoints_right, matches, Hl, Hr);
    cout << " done";

    // render the output
    render(height_left, width_left, img_left, height_right, width_right, img_right, Hl, Hr, "panorama.png");
    cout  << "Complex rendering done" << endl << endl << endl;
}
