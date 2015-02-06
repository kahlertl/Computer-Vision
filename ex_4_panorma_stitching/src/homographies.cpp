#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/features2d/features2d.hpp>  // DMatch

#include "panorama.hpp"

using namespace std;
using namespace cv;


void findHomographyLR(const vector<KeyPoint>& keypoints_left, const vector<KeyPoint>& keypoints_right,
                      const vector<DMatch>& matches, Mat& Hl, Mat& Hr)
{

    // find "usual" hopmgraphy: left->right
    vector<Point2d> points_left;
    vector<Point2d> points_right;

    for (int i = 0; i < matches.size(); i++) {
        points_left.push_back(keypoints_left[matches[i].queryIdx].pt);
        points_right.push_back(keypoints_right[matches[i].trainIdx].pt);
    }

    Mat H = findHomography(points_left, points_right, CV_RANSAC);

    // try to estimate the "middle one"
    points_left.clear();
    points_right.clear();

    for (int i = 0; i < matches.size(); i++) {
        double x0, y0;
        double x, y;

        // x0 = matches[i].xl; 
        // y0 = matches[i].yl;

        x0 = keypoints_left[matches[i].queryIdx].pt.x;
        y0 = keypoints_left[matches[i].queryIdx].pt.y;

        ht(x0, y0, H, &x, &y);

        x = (x + x0) / 2.;
        y = (y + y0) / 2.;

        points_left.push_back(Point2d(x0, y0));
        points_right.push_back(Point2d(x, y));
    }

    Hl = findHomography(points_left, points_right, CV_RANSAC);

    // iterate
    for (int it = 0; it < 4; it++) {
        // transform the left points and keep the right ones
        points_left.clear();
        points_right.clear();
        for (int i = 0 ; i < matches.size(); i++) {
            double x, y;

            ht(
                keypoints_left[matches[i].queryIdx].pt.x,
                keypoints_left[matches[i].queryIdx].pt.y,
                Hl, &x, &y
            );

            points_left.push_back(Point2d(x, y));
            points_right.push_back(keypoints_right[matches[i].trainIdx].pt);
        }
        // find the right homography
        Hr = findHomography(points_right, points_left, CV_RANSAC);

        // transform the right points and keep the left ones
        points_left.clear();
        points_right.clear();
        for (int i = 0 ; i < matches.size(); i++) {
            double x, y;

            ht(
                keypoints_right[matches[i].trainIdx].pt.x,
                keypoints_right[matches[i].trainIdx].pt.y,
                Hr, &x, &y
            );

            points_left.push_back(Point2d(keypoints_left[matches[i].queryIdx].pt.x, keypoints_left[matches[i].queryIdx].pt.y));
            points_right.push_back(Point2d(x, y));
        }
        // find the right homography
        Hl = findHomography(points_left, points_right, CV_RANSAC);
    }
}