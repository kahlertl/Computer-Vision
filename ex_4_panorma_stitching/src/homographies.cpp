#include <stdio.h>
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <time.h>
#include <fstream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
#include "opencv2/calib3d/calib3d.hpp"

#include "ps.h"

void my_homographies(vector<MATCH> matches, Mat &Hl, Mat &Hr)
{

    // find "usual" hopmgraphy: left->right
    vector<Point2d> v1;
    vector<Point2d> v2;
    v1.clear();
    v2.clear();

    for (unsigned int i = 0; i < matches.size(); i++) {
        v1.push_back(Point2d(matches[i].xl, matches[i].yl));
        v2.push_back(Point2d(matches[i].xr, matches[i].yr));
    }

    Mat H = findHomography(v1, v2, CV_RANSAC);

    // try to estimate the "middle one"
    v1.clear();
    v2.clear();
    for (unsigned int i = 0; i < matches.size(); i++) {
        double x0, y0, x, y;
        x0 = matches[i].xl;
        y0 = matches[i].yl;
        ht(x0, y0, H, &x, &y);
        x = (x + x0) / 2.;
        y = (y + y0) / 2.;
        v1.push_back(Point2d(x0, y0));
        v2.push_back(Point2d(x, y));
    }

    Hl = findHomography(v1, v2, CV_RANSAC);

    // iterate
    for (int it = 0; it < 4; it++) {
        // transform the left points and keep the right ones
        v1.clear();
        v2.clear();
        for (unsigned int i = 0 ; i < matches.size(); i++) {
            double x, y;
            ht(matches[i].xl, matches[i].yl, Hl, &x, &y);
            v1.push_back(Point2d(x, y));
            v2.push_back(Point2d(matches[i].xr, matches[i].yr));
        }
        // find the right homography
        Hr = findHomography(v2, v1, CV_RANSAC);

        // transform the right points and keep the left ones
        v1.clear();
        v2.clear();
        for (unsigned int i = 0 ; i < matches.size(); i++) {
            double x, y;
            ht(matches[i].xr, matches[i].yr, Hr, &x, &y);
            v1.push_back(Point2d(matches[i].xl, matches[i].yl));
            v2.push_back(Point2d(x, y));
        }
        // find the right homography
        Hl = findHomography(v1, v2, CV_RANSAC);
    }
}