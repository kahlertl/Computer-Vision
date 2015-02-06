#ifndef __PANST
#define __PANST

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>  // KeyPoint, DMatch

// saves.cpp: save_double_as_image() will rescale the input to [0, 255]
#define RESCALE_MINMAX

#ifndef VERBOSE
    const bool verbose = true;
#else
    const bool verbose = false;
#endif

void marriageMatch(const cv::Mat& descriptors_left,
                   const cv::Mat& descriptors_right,
                   cv::DescriptorMatcher& matcher,
                   const int k,
                   cv::vector<cv::DMatch>& matches);

void suppressNonMax(int width, int height, std::vector<cv::KeyPoint>& keypoints, int radius);

// 
// Save methods
// 

void save_double_as_image(int height, int width, double *array, const char *name);

void save_keypoints_as_image(const cv::Mat& image, const std::vector<cv::KeyPoint>& keypoints, const char* filename);


inline void ht(double x0, double y0, cv::Mat &H, double *x, double *y)
{
    cv::Mat point1 = (cv::Mat_<double>(3, 1) << x0, y0, 1.);
    cv::Mat point2 = H * point1;
    *x = point2.at<double>(0, 0) / point2.at<double>(2, 0);
    *y = point2.at<double>(1, 0) / point2.at<double>(2, 0);
}

void render(int heightl, int widthl, cv::Mat &imgl,
            int heightr, int widthr, cv::Mat &imgr,
            cv::Mat Hl, cv::Mat Hr, const char *name);

void findHomographyLR(const std::vector<cv::KeyPoint>& keypoints_left,
                      const std::vector<cv::KeyPoint>& keypoints_right,
                      const std::vector<cv::DMatch>& matches,
                      cv::Mat& Hl, cv::Mat& Hr);

#endif