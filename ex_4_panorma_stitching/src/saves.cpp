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

#include "panorma.hpp"

using namespace std;
using namespace cv;

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


void save_keypoints_as_image(const Mat& image, const vector<KeyPoint>& keypoints, const char* filename)
{
    Mat out;
    Scalar color = { 255, 0, 0 };

    drawKeypoints(image, keypoints, out, color, DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    // store image
    imwrite(filename, out);
}