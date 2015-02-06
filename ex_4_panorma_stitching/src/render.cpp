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
#include "opencv2/calib3d/calib3d.hpp"

#include "panorma.hpp"

double get_weight(int i, int j, int height, int width)
{
    double dist = i;
    if (height - 1 - i < dist) dist = height - 1 - i;
    if (j < dist) dist = j;
    if (width - 1 - j < dist) dist = width - 1 - j;
    dist = exp((dist - 20.) / 5.);
    return dist / (1. + dist);
}

void render(int heightl, int widthl, Mat &imgl,
            int heightr, int widthr, Mat &imgr,
            Mat Hl, Mat Hr, const char *name)
{

    // sizes
    double x, y;
    double xmin = widthl;
    double xmax = 0.;
    double ymin = heightl;
    double ymax = 0.;

    // left image -- transform corners to estimate sizes
    ht(0., 0., Hl, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
    ht(widthl - 1., 0., Hl, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
    ht(0., heightl - 1., Hl, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
    ht(widthl - 1., heightl - 1., Hl, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;

    // right image -- transform corners to estimate sizes
    ht(0., 0., Hr, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
    ht(widthr - 1., 0., Hr, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
    ht(0., heightr - 1., Hr, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;
    ht(widthr - 1., heightr - 1., Hr, &x, &y);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
    if (y < ymin) ymin = y;
    if (y > ymax) ymax = y;

    double shifty = -ymin + 2.5;
    double shiftx = -xmin + 2.5;
    int height = (int)(ymax - ymin + 5.);
    int width = (int)(xmax - xmin + 5.);

    double *rr = new double[height * width];
    double *gg = new double[height * width];
    double *bb = new double[height * width];
    double *ww = new double[height * width];
    unsigned char *pimgl = imgl.ptr(0);
    unsigned char *pimgr = imgr.ptr(0);

    for (int i = 0; i < height * width; i++) {
        rr[i] = gg[i] = bb[i] = 0.;
        ww[i] = 0.001;
    }

    // stamp the left image
    for (int i = 0; i < heightl; i++) {
        for (int j = 0; j < widthl; j++) {
            double x, y;
            ht(j, i, Hl, &x, &y);
            x += shiftx;
            y += shifty;
            int jm = (int)x;
            int jp = jm + 1;
            int im = (int)y;
            int ip = im + 1;
            double wjm = (1. - x + jm);
            double wjp = (1. - wjm);
            double wim = (1. - y + im);
            double wip = (1. - wim);
            double gw = get_weight(i, j, heightl, widthl);
            rr[im * width + jm] += pimgl[(i * widthl + j) * 3 + 0] * wim * wjm * gw;
            gg[im * width + jm] += pimgl[(i * widthl + j) * 3 + 1] * wim * wjm * gw;
            bb[im * width + jm] += pimgl[(i * widthl + j) * 3 + 2] * wim * wjm * gw;
            ww[im * width + jm] += wim * wjm * gw;
            rr[im * width + jp] += pimgl[(i * widthl + j) * 3 + 0] * wim * wjp * gw;
            gg[im * width + jp] += pimgl[(i * widthl + j) * 3 + 1] * wim * wjp * gw;
            bb[im * width + jp] += pimgl[(i * widthl + j) * 3 + 2] * wim * wjp * gw;
            ww[im * width + jp] += wim * wjp * gw;
            rr[ip * width + jm] += pimgl[(i * widthl + j) * 3 + 0] * wip * wjm * gw;
            gg[ip * width + jm] += pimgl[(i * widthl + j) * 3 + 1] * wip * wjm * gw;
            bb[ip * width + jm] += pimgl[(i * widthl + j) * 3 + 2] * wip * wjm * gw;
            ww[ip * width + jm] += wip * wjm * gw;
            rr[ip * width + jp] += pimgl[(i * widthl + j) * 3 + 0] * wip * wjp * gw;
            gg[ip * width + jp] += pimgl[(i * widthl + j) * 3 + 1] * wip * wjp * gw;
            bb[ip * width + jp] += pimgl[(i * widthl + j) * 3 + 2] * wip * wjp * gw;
            ww[ip * width + jp] += wip * wjp * gw;
        }
    }

    // stamp the right image
    for (int i = 0; i < heightr; i++) {
        for (int j = 0; j < widthr; j++) {
            double x, y;
            ht(j, i, Hr, &x, &y);
            x += shiftx;
            y += shifty;
            int jm = (int)x;
            int jp = jm + 1;
            int im = (int)y;
            int ip = im + 1;
            double wjm = (1. - x + jm);
            double wjp = (1. - wjm);
            double wim = (1. - y + im);
            double wip = (1. - wim);
            double gw = get_weight(i, j, heightr, widthr);
            rr[im * width + jm] += pimgr[(i * widthr + j) * 3 + 0] * wim * wjm * gw;
            gg[im * width + jm] += pimgr[(i * widthr + j) * 3 + 1] * wim * wjm * gw;
            bb[im * width + jm] += pimgr[(i * widthr + j) * 3 + 2] * wim * wjm * gw;
            ww[im * width + jm] += wim * wjm * gw;
            rr[im * width + jp] += pimgr[(i * widthr + j) * 3 + 0] * wim * wjp * gw;
            gg[im * width + jp] += pimgr[(i * widthr + j) * 3 + 1] * wim * wjp * gw;
            bb[im * width + jp] += pimgr[(i * widthr + j) * 3 + 2] * wim * wjp * gw;
            ww[im * width + jp] += wim * wjp * gw;
            rr[ip * width + jm] += pimgr[(i * widthr + j) * 3 + 0] * wip * wjm * gw;
            gg[ip * width + jm] += pimgr[(i * widthr + j) * 3 + 1] * wip * wjm * gw;
            bb[ip * width + jm] += pimgr[(i * widthr + j) * 3 + 2] * wip * wjm * gw;
            ww[ip * width + jm] += wip * wjm * gw;
            rr[ip * width + jp] += pimgr[(i * widthr + j) * 3 + 0] * wip * wjp * gw;
            gg[ip * width + jp] += pimgr[(i * widthr + j) * 3 + 1] * wip * wjp * gw;
            bb[ip * width + jp] += pimgr[(i * widthr + j) * 3 + 2] * wip * wjp * gw;
            ww[ip * width + jp] += wip * wjp * gw;
        }
    }

    Mat out(height, width, CV_8UC3);
    unsigned char *pout = out.ptr(0);
    for (int i = 0, ii = 0; i < height * width; i++) {
        pout[ii++] = (unsigned char)(rr[i] / ww[i]);
        pout[ii++] = (unsigned char)(gg[i] / ww[i]);
        pout[ii++] = (unsigned char)(bb[i] / ww[i]);
    }
    imwrite(name, out);

    delete rr;
    delete gg;
    delete bb;
    delete ww;
}
