#include <stdio.h>
#include <iostream>
#include <set>
#include "ps.h"

using namespace std;
using namespace cv;

vector<KEYPOINT> local_maxima(int height, int width, double *e, int radius, const char *name)
{
    // separable maximum filter
    // 
    double *hmax = new double[height * width];
    double *vmax = new double[height * width];

    // Use multiset as max heap (std::multiheap keeps its element in a strict
    // week ordering).
    // set.rbegin() returns a pointer to the maxial element in the heap
    multiset<double> set;

    // horizontal
    for (int i = 0; i < height; i++) {
        set.clear();
        // prefill the multiset
        for (int j = 0; j < radius; j++) set.insert(e[i * width + j]);
        // left border
        for (int j = 0; j < radius + 1; j++) {
            set.insert(e[i * width + j + radius]);
            hmax[i * width + j] = *(set.rbegin());
        }
        // inside
        for (int j = radius + 1; j < width - radius; j++) {
            // delete first element that is out of scope
            set.erase(set.find(e[i * width + j - radius - 1]));
            set.insert(e[i * width + j + radius]);
            hmax[i * width + j] = *(set.rbegin());
        }
        // right border
        for (int j = width - radius; j < width; j++) {
            set.erase(set.find(e[i * width + j - radius - 1]));
            hmax[i * width + j] = *(set.rbegin());
        }
    }
    // vertical
    for (int j = 0; j < width; j++) {
        set.clear();
        // prefill the multiset
        for (int i = 0; i < radius; i++) set.insert(hmax[i * width + j]);
        // top border
        for (int i = 0; i < radius + 1; i++) {
            set.insert(hmax[(i + radius)*width + j]);
            vmax[i * width + j] = *(set.rbegin());
        }
        // inside
        for (int i = radius + 1; i < height - radius; i++) {
            set.erase(set.find(hmax[(i - radius - 1)*width + j]));
            set.insert(hmax[(i + radius)*width + j]);
            vmax[i * width + j] = *(set.rbegin());
        }
        // bottom border
        for (int i = height - radius; i < height; i++) {
            set.erase(set.find(hmax[(i - radius - 1)*width + j]));
            vmax[i * width + j] = *(set.rbegin());
        }
    }

#ifdef SAVE_ALL
    save_double_as_image(height, width, vmax, (string("m") + string(name) + string(".png")).c_str());
#endif

    // find local maxima
    vector<KEYPOINT> ret;
    ret.clear();

    for (int i = radius + 2; i < height - radius - 2; i++) {
        for (int j = radius + 2; j < width - radius - 2; j++) {
            if (e[i * width + j] == vmax[i * width + j]) {
                KEYPOINT point;
                point.x = j;
                point.y = i;
                point.value = vmax[i * width + j];
                ret.push_back(point);
            }
        }
    }

    delete hmax;
    delete vmax;

    return ret;
}


ostream& operator<<(ostream& os, const multiset<float>& set)
{
    for (multiset<float>::iterator i = set.begin(); i != set.end(); ++i) {
        os << (*i) << " ";
    }

    return os;
}

void suppressNonMax(int width, int height, std::vector<cv::KeyPoint>& keypoints, int radius)
{
    // create a matrix of the qualities
    Mat responses = Mat::zeros(width, height, CV_32FC1);

    for (int i = 0; i < keypoints.size(); i++) {
        responses.at<float>(keypoints[i].pt) = keypoints[i].response;
    }

    // separable maximum filter
    // 
    Mat hmax = Mat(width, height, CV_32FC1);
    Mat vmax = Mat(width, height, CV_32FC1);

    // Use multiset as max heap (std::multiheap keeps its element in a strict
    // week ordering).
    // set.rbegin() returns a pointer to the maxial element in the heap
    multiset<float> set;

    // horizontal
    // 
    for (int row = 0; row < height; row++) {
        set.clear();

        // prefill the multiset with values of pixels that have no
        // left neighbor
        for (int col = 0; col < radius; col++) {
            set.insert(responses.at<float>(row, col));
        }

        // left border
        for (int col = 0; col < radius + 1; col++) {
            // insert next right element in scope
            set.insert(responses.at<float>(row, col + radius));
            // write local maximum in the row
            hmax.at<float>(row, col) = *(set.rbegin());
        }

        // inside
        for (int col = radius + 1; col < width - radius; col++) {
            // delete first element that is out of scope
            set.erase(set.find(responses.at<float>(row, col - radius - 1)));
            set.insert(responses.at<float>(row, col + radius));
            hmax.at<float>(row, col) = *(set.rbegin());
        }

        // right border
        // we cannot insert right neighbors anymore
        for (int col = width - radius; col < width; col++) {
            set.erase(set.find(responses.at<float>(row, col - radius - 1)));
            hmax.at<float>(row, col) = *(set.rbegin());
        }
    }

    // vertical
    // 
    // the same as for the horizontal run, but use the already
    // computed hmax matrix as input
    for (int col = 0; col < width; col++) {
        set.clear();

        // prefill the multiset
        for (int row = 0; row < radius; row++) {
            set.insert(hmax.at<float>(row, col));
        }

        // top border
        for (int row = 0; row < radius + 1; row++) {
            set.insert(hmax.at<float>(row + radius, col));
            vmax.at<float>(row, col) = *(set.rbegin());
        }

        // inside
        for (int row = radius + 1; row < height - radius; row++) {
            set.erase(set.find(hmax.at<float>(row - radius - 1, col)));
            set.insert(hmax.at<float>(row + radius, col));
            vmax.at<float>(row, col) = *(set.rbegin());
        }

        // bottom border
        for (int row = height - radius; row < height; row++) {
            set.erase(set.find(hmax.at<float>(row - radius - 1, col)));
            vmax.at<float>(row, col) = *(set.rbegin());
        }
    }

    // index pointing to the last element in the
    // vector that must kept
    int j = 0;

    for (int i = 0; i < keypoints.size(); i++) {
        if (vmax.at<float>(keypoints[i].pt) == keypoints[i].response) {
            keypoints[j++] = keypoints[i];
        }
    }

    keypoints.resize(j);
}