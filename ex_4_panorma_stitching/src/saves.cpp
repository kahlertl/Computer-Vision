// standard stuff
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

#include "ps.h"

#define RESCALE_MINMAX

void save_double_as_image(int height, int width, double *array, const char *name)
{

#ifdef RESCALE_MINMAX

    double min = array[0];
    double max = array[0];
    for (int i = 1; i < height * width; i++) {
        if (array[i] > max) max = array[i];
        if (array[i] < min) min = array[i];
    }

    Mat A(height, width, CV_8UC1);
    unsigned char *img = A.ptr(0);
    for (int i = 0; i < height * width; i++)
        img[i] = (unsigned char)((array[i] - min) * 255. / (max - min) + 0.5);
    imwrite(name, A);

#else

    double mean = 0.;
    double var = 0.;
    for (int i = 1; i < height * width; i++) {
        mean += array[i];
        var += array[i] * array[i];
    }
    mean /= height * width;
    var = sqrt(var / (height * width) - mean * mean);

    Mat A(height, width, CV_8UC1);
    unsigned char *img = A.ptr(0);
    for (int i = 0; i < height * width; i++) {
        double tmp = 128. + (array[i] - mean) / var * 127. + 0.5;
        if (tmp < 0.) tmp = 0.;
        if (tmp > 255.) tmp = 255.;
        img[i] = (unsigned char)tmp;
    }
    imwrite(name, A);

#endif

}

void save_matches_as_image(int heightl, int widthl, unsigned char *imgl,
                           int heightr, int widthr, unsigned char *imgr,
                           vector<MATCH> matches, const char *name)
{
    int width = widthl > widthr ? widthl : widthr;
    int height = heightl + heightr;
    Mat A(height, width, CV_8UC3);
    unsigned char *img = A.ptr(0);

    for (int i = 0; i < width * height * 3; i++) img[i] = 0;
    for (int i = 0; i < heightl; i++) {
        for (int j = 0; j < widthl; j++) {
            img[(i * width + j) * 3] = imgl[(i * widthl + j) * 3];
            img[(i * width + j) * 3 + 1] = imgl[(i * widthl + j) * 3 + 1];
            img[(i * width + j) * 3 + 2] = imgl[(i * widthl + j) * 3 + 2];
        }
    }
    for (int i = 0; i < heightr; i++) {
        for (int j = 0; j < widthr; j++) {
            img[((i + heightl)*width + j) * 3] = imgr[(i * widthr + j) * 3];
            img[((i + heightl)*width + j) * 3 + 1] = imgr[(i * widthr + j) * 3 + 1];
            img[((i + heightl)*width + j) * 3 + 2] = imgr[(i * widthr + j) * 3 + 2];
        }
    }

    for (unsigned int i = 0; i < matches.size(); i++) {
        int x0 = (int)(matches[i].xl + 0.5);
        int y0 = (int)(matches[i].yl + 0.5);
        int x1 = (int)(matches[i].xr + 0.5);
        int y1 = (int)(matches[i].yr + 0.5);
        line(A, Point(x0, y0), Point(x1, y1 + heightl), CV_RGB(0, 0, 0));
    }

    imwrite(name, A);
}

void save_keypoints_as_image(int height, int width, unsigned char *img,
                             vector<KEYPOINT> points, const char *name)
{
    Mat A(height, width, CV_8UC3);
    unsigned char *out = A.ptr(0);
    memcpy((void *)out, (const void *)img, width * height * 3);
    for (unsigned int i = 0; i < points.size(); i++) {
        int x = (int)(points[i].x + 0.5);
        int y = (int)(points[i].y + 0.5);
        circle(A, Point(x, y), 5, CV_RGB(0, 0, 0));
    }

    imwrite(name, A);
}
