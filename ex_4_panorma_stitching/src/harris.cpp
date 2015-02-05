#include <stdio.h>
#include <iostream>
#include "ps.h"

using namespace std;

vector<KEYPOINT> harris(int height, int width, unsigned char *img,
                        int wsize_sum, int wsize_loc, const char *name)
{

    // compute a,b and c coeffitients
    double *a = new double[height * width];
    double *b = new double[height * width];
    double *c = new double[height * width];

    // zero borders (no gradients -> no detection)
    for (int i = 0; i < height; i++) {
        a[i * width] = b[i * width] = c[i * width] =
                                          a[i * width + width - 1] = b[i * width + width - 1] = c[i * width + width - 1] = 0.;
    }
    for (int j = 0; j < width; j++) {
        a[j] = b[j] = c[j] =
                          a[width * (height - 1) + j] = b[width * (height - 1) + j] = c[width * (height - 1) + j] = 0.;
    }

    // loop over inside
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            // vertical gradient
            double rgrad =
                (double)img[INDEX(i - 1, j + 1, 0)] + img[INDEX(i, j + 1, 0)] + img[INDEX(i + 1, j + 1, 0)] -
                img[INDEX(i - 1, j - 1, 0)] - img[INDEX(i, j - 1, 0)] - img[INDEX(i + 1, j - 1, 0)];
            double ggrad =
                (double)img[INDEX(i - 1, j + 1, 1)] + img[INDEX(i, j + 1, 1)] + img[INDEX(i + 1, j + 1, 1)] -
                img[INDEX(i - 1, j - 1, 1)] - img[INDEX(i, j - 1, 1)] - img[INDEX(i + 1, j - 1, 1)];
            double bgrad =
                (double)img[INDEX(i - 1, j + 1, 2)] + img[INDEX(i, j + 1, 2)] + img[INDEX(i + 1, j + 1, 2)] -
                img[INDEX(i - 1, j - 1, 2)] - img[INDEX(i, j - 1, 2)] - img[INDEX(i + 1, j - 1, 2)];
            double vgrad = sqrt(rgrad * rgrad + ggrad * ggrad + bgrad * bgrad);
            // horizontal gradient
            rgrad =
                (double)img[INDEX(i + 1, j - 1, 0)] + img[INDEX(i + 1, j, 0)] + img[INDEX(i + 1, j + 1, 0)] -
                img[INDEX(i - 1, j - 1, 0)] - img[INDEX(i - 1, j, 0)] - img[INDEX(i - 1, j + 1, 0)];
            ggrad =
                (double)img[INDEX(i + 1, j - 1, 1)] + img[INDEX(i + 1, j, 1)] + img[INDEX(i + 1, j + 1, 1)] -
                img[INDEX(i - 1, j - 1, 1)] - img[INDEX(i - 1, j, 1)] - img[INDEX(i - 1, j + 1, 1)];
            bgrad =
                (double)img[INDEX(i + 1, j - 1, 2)] + img[INDEX(i + 1, j, 2)] + img[INDEX(i + 1, j + 1, 2)] -
                img[INDEX(i - 1, j - 1, 2)] - img[INDEX(i - 1, j, 2)] - img[INDEX(i - 1, j + 1, 2)];
            double hgrad = sqrt(rgrad * rgrad + ggrad * ggrad + bgrad * bgrad);

            a[i * width + j] = hgrad * hgrad;
            b[i * width + j] = hgrad * vgrad;
            c[i * width + j] = vgrad * vgrad;
        }
    }

    #ifdef SAVE_ALL
        save_double_as_image(height, width, a, (string("a") + string(name) + string(".png")).c_str());
        save_double_as_image(height, width, b, (string("b") + string(name) + string(".png")).c_str());
        save_double_as_image(height, width, c, (string("c") + string(name) + string(".png")).c_str());
    #endif

    // a, b, c are computed, integrate now
    mean_filter(height, width, a, wsize_sum);
    mean_filter(height, width, b, wsize_sum);
    mean_filter(height, width, c, wsize_sum);

    #ifdef SAVE_ALL
        save_double_as_image(height, width, a, (string("aa") + string(name) + string(".png")).c_str());
        save_double_as_image(height, width, b, (string("bb") + string(name) + string(".png")).c_str());
        save_double_as_image(height, width, c, (string("cc") + string(name) + string(".png")).c_str());
    #endif

    // cornerness
    double *e = new double[height * width];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            e[i * width + j] = a[i * width + j] * c[i * width + j] - b[i * width + j] * b[i * width + j]
                               - 0.04 * (a[i * width + j] + c[i * width + j]) * (a[i * width + j] + c[i * width + j]);
        }
    }

    #ifdef SAVE_ALL
        save_double_as_image(height, width, e, (string("e") + string(name) + string(".png")).c_str());
    #endif

    vector<KEYPOINT> points = local_maxima(height, width, e, wsize_loc, name);

    // filter out the "obviously bad" ones -- those with the quality less than the mean
    double sum = 0;
    for (unsigned int i = 0; i < points.size(); i++) sum += points[i].value;
    sum /= points.size();
    vector<KEYPOINT> ret;
    ret.clear();
    for (unsigned int i = 0; i < points.size(); i++) {
        if (points[i].value > sum) ret.push_back(points[i]);
    }

    // free memory
    delete a;
    delete b;
    delete c;
    delete e;

    return ret;
}
