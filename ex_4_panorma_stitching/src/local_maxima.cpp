#include <stdio.h>
#include <iostream>
#include <set>

#include "panorama.hpp"

using namespace std;
using namespace cv;


ostream& operator<<(ostream& os, const multiset<float>& set)
{
    for (multiset<float>::iterator i = set.begin(); i != set.end(); ++i) {
        os << (*i) << " ";
    }

    return os;
}


/**
 * Search for local maxima in a vector of keypoints and remove non-maximum keypoints
 * 
 * @param width     width of the image
 * @param height    height of the image
 * @param keypoints
 * @param radius    radius of the windows in which non-maximum keypoints are suppressed
 */
void suppressNonMax(int width, int height, std::vector<cv::KeyPoint>& keypoints, int radius)
{
    // create a matrix of the qualities
    Mat responses = Mat::zeros(width, height, CV_32FC1);

    // prefill quality matrix
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

    // Remove all non-maximum elements
    for (int i = 0; i < keypoints.size(); i++) {
        if (vmax.at<float>(keypoints[i].pt) == keypoints[i].response) {
            keypoints[j++] = keypoints[i];
        }
    }

    keypoints.resize(j);
}