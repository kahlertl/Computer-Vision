// standard stuff
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <limits>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>  // KeyPoint
#include <opencv2/nonfree/features2d.hpp>     // SurfFeatureDetector 

// the header for all functions/macros of this project
#include "ps.h"

using namespace std;
using namespace cv;

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

    Mat img_right = imread(argv[2], CV_LOAD_IMAGE_COLOR);
    if (img_right.empty()) {
        cerr << "Can not read " << argv[2] << endl;
        return 1;
    }

    // Convert to grayscale image
    Mat gray_left;
    Mat gray_right;

    cvtColor(img_left,  gray_left,  CV_BGR2GRAY);
    cvtColor(img_right, gray_right, CV_BGR2GRAY);

    // faust regel for window sizes :-)
    // + 0.5 for round up
    int wsize_sum   = (gray_left.cols + gray_right.cols + gray_left.rows + gray_right.rows) / 1300. + 0.5;
    int wsize_loc   = (gray_left.cols + gray_right.cols + gray_left.rows + gray_right.rows) / 500. + 0.5;
    int wsize_match = (gray_left.cols + gray_right.cols + gray_left.rows + gray_right.rows) / 90. + 0.5;

    if (wsize_sum < 1) wsize_sum = 1;
    if (wsize_loc < 2) wsize_loc = 2;
    if (wsize_match < 5) wsize_match = 5;
    if (wsize_match > 40) wsize_match = 40;

    cout << "Images loaded. wsize_sum=" << wsize_sum
         << ", wsize_loc="              << wsize_loc
         << ", wsize_match="            << wsize_match
         << endl;

    // Feature / keypoint detection
    // 
    cout << "Detect keypoints ..." << endl;

    vector<KeyPoint> keypoints_left, keypoints_right;

    int minHessian = 600;
    SurfFeatureDetector detector(minHessian);

    detector.detect(gray_left, keypoints_left);
    detector.detect(gray_right, keypoints_right);

    cout << "Non maximum suppression ..." << endl;

    // non-maximum-suppression
    // 
    suppressNonMax(gray_left.cols,  gray_left.rows,  keypoints_left,  gray_left.cols  * 0.01); // TODO parameter for this scaling factor
    suppressNonMax(gray_right.cols, gray_right.rows, keypoints_right, gray_right.cols * 0.01);

    #ifdef SAVE_ALL
        save_keypoints_as_image(gray_left,  keypoints_left,  "keypointsL.png");
        save_keypoints_as_image(gray_right, keypoints_right, "keypointsR.png");
    #endif

    // Calculate descriptors (alias feature vectors)
    // 
    SurfDescriptorExtractor extractor;
    Mat descriptors_left, descriptors_right;

    cout << "Compute feature descriptors ..." << endl;

    extractor.compute(gray_left,  keypoints_left,  descriptors_left);
    extractor.compute(gray_right, keypoints_right, descriptors_right);

    
    // Matching
    // 
    FlannBasedMatcher matcher;
    // BFMatcher matcher;
    // vector<DMatch> matches;
    vector<DMatch> matches;

    cout << "Matching ..." << endl;

    // matcher.match(descriptors_left, descriptors_right, matches);

    marriageMatch(descriptors_left, descriptors_right, matcher, 10, matches);

    #ifdef SAVE_ALL
        Mat img_matches;
        drawMatches(
            gray_left,  keypoints_left,              // left image with its keypoints
            gray_right, keypoints_right,             // right image with its keypoints
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


    // Apply quality threshold on the matches
    // 
    double max_dist = 0;
    double min_dist = numeric_limits<double>::max();

    // Quick calculation of max and min distances between keypoints
    for (int i = 0; i < matches.size(); i++) {
        double dist = matches[i].distance;

        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }
  
    cout << "Max dist: " << max_dist << endl;
    cout << "Min dist: " << min_dist << endl;
  
    int j = 0;
    for (int i = 0; i < matches.size(); i++) {
        if (matches[i].distance <= max(8 * min_dist, 0.02)) {
            matches[j++] = matches[i];
        }
    }
    matches.resize(j);

    #ifdef SAVE_ALL
        Mat img_matches_nonmax;
        drawMatches(
            gray_left,  keypoints_left,              // left image with its keypoints
            gray_right, keypoints_right,             // right image with its keypoints
            matches,                                 // matches between the keypoints
            img_matches_nonmax,                      // output image
            Scalar::all(-1),                         // color of matches
            Scalar::all(-1),                         // color of single points
            vector<char>(),                          // mask determining which matches are drawn. If empty
                                                     // all matches are drawn 
            DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS // Single keypoints will not be drawn
        );
        imwrite("matches-maxdist.png", img_matches_nonmax);
    #endif


    // // homography -- OpenCV implementation
    // cout  << "Start standard RANSAC ..." << endl;
    // vector<Point2d> points_left;
    // vector<Point2d> points_right;
    // for (int i = 0; i < matches.size(); i++) {
    //     points_left.push_back(keypoints_left[matches[i].queryIdx].pt);
    //     points_right.push_back(keypoints_right[matches[i].trainIdx].pt);
    // }
    // Mat H = findHomography(points_left, points_right, CV_RANSAC);
    // Mat Hi = H.inv();
    // cout << " done";

    // // render the output
    // Mat imglt = img_right.clone();
    // warpPerspective(img_left, imglt, H, imglt.size(), INTER_LINEAR);
    // // imglt = imglt * 0.5 + img_right * 0.5;
    // imwrite("warpedL.png", imglt);

    // Mat imgrt = img_left.clone();
    // warpPerspective(img_right, imgrt, Hi, imgrt.size(), INTER_LINEAR);
    // // imgrt = imgrt * 0.5 + img_left * 0.5;
    // imwrite("warpedR.png", imgrt);

    // cout  << "Simple rendering done" << endl;

    // find two homographies
    Mat Hl, Hr;
    if (verbose) { cout  << "Start another RANSAC ... "; }
    my_homographies(keypoints_left, keypoints_right, matches, Hl, Hr);
    if (verbose) { cout << "done" << endl; }

    // render the output
    if (verbose) { cout << "Render ... " << endl; }
    render(img_left.rows, img_left.cols, img_left, img_right.rows, img_right.cols, img_right, Hl, Hr, "panorama-maxdist.png");
    cout << "done" << endl;


    vector<Point2d> points_left;
    vector<Point2d> points_right;

    for (int i = 0; i < matches.size(); i++) {
        points_left.push_back(keypoints_left[matches[i].queryIdx].pt);
        points_right.push_back(keypoints_right[matches[i].trainIdx].pt);
    }

    // // Find the Homography Matrix
    // Mat H = findHomography(points_left, points_right, CV_RANSAC);

    // // Use the Homography Matrix to warp the images
    // cv::Mat panorama;

    // warpPerspective(img_left,panorama,H,cv::Size(img_left.cols + img_right.cols, img_left.rows));

    // cv::Mat half(panorama, cv::Rect(0, 0, img_right.cols, img_right.rows));
    // img_right.copyTo(half);

    // imwrite("panorama.png", panorama);

    return 0;
}
