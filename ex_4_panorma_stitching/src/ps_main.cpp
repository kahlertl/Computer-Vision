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

    if (verbose) {
        save_keypoints_as_image(gray_left,  keypoints_left,  "keypointsL.png");
        save_keypoints_as_image(gray_right, keypoints_right, "keypointsR.png");
    }

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
    // BFMatcher matcher;  // Brute-Force matcher
    vector<DMatch> matches;

    cout << "Matching ..." << endl;

    // matcher.match(descriptors_left, descriptors_right, matches);
    marriageMatch(descriptors_left, descriptors_right, matcher, 10, matches);

    if (verbose) {
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
    }


    // Apply quality threshold on the matches
    // 
    double max_dist = 0;
    double min_dist = numeric_limits<double>::max();

    if (verbose) { cout << "Remove bad matches ..." << endl; }

    // Quick calculation of max and min distances between keypoints
    for (int i = 0; i < matches.size(); i++) {
        double dist = matches[i].distance;

        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }
  
    if (verbose){
        cout << "  Max dist: " << max_dist << endl
             << "  Min dist: " << min_dist << endl;
    }
  
    // Removes all matches that have a heigher distance than
    // the configured threshold.
    int j = 0;
    for (int i = 0; i < matches.size(); i++) {
        // We use a constant 0.02, because if we have found a nearly 
        // perfect match, all other matches would be removed.
        if (matches[i].distance <= max(8 * min_dist, 0.02)) {
            matches[j++] = matches[i];
        }
    }
    matches.resize(j);

    if (verbose) {
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
    }

    if (verbose) { cout << "Done" << endl; }


    // find two homographies
    // 
    if (verbose) { cout  << "Start RANSAC ... "; }
    Mat Hl, Hr;
    findHomographyLR(keypoints_left, keypoints_right, matches, Hl, Hr);
    if (verbose) { cout << "done" << endl; }

    // render the output
    // 
    if (verbose) { cout << "Render ... " << endl; }
    render(img_left.rows, img_left.cols, img_left, img_right.rows, img_right.cols, img_right, Hl, Hr, "panorama-maxdist.png");
    if (verbose) { cout << "done" << endl; }

    return 0;
}