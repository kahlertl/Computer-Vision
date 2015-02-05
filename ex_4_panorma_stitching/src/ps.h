#ifndef __PANST
#define __PANST

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>  // KeyPoint, DMatch

// save all intermediate results as images
#define SAVE_ALL

#ifndef VERBOSE
    const bool verbose = true;
#else
    const bool verbose = false;
#endif

#define INDEX(i,j,c) ((((i)*width)+(j))*3+(c))

typedef struct {
    double x;
    double y;
    double value;
} KEYPOINT;

typedef struct {
    double xl;
    double yl;
    double xr;
    double yr;
    double value;
} MATCH;

std::vector<KEYPOINT> harris(int height, int width, unsigned char *img,
                             int wsize_sum, int wsize_loc, const char *name);


std::vector<MATCH> matching(int heightl, int widthl, unsigned char *imgl,
                            int heightr, int widthr, unsigned char *imgr,
                            std::vector<KEYPOINT>pointsl, std::vector<KEYPOINT>pointsr, int wsize);

void marriageMatch(const std::vector<std::vector<cv::DMatch>>& acceptor_table,
                   const std::vector<std::vector<cv::DMatch>>& proposor_table,
                   const int k,
                   std::vector<cv::DMatch>& matches);

void mean_filter(int height, int width, double *a, int wsize);

std::vector<KEYPOINT> local_maxima(int height, int width, double *e, int wsize, const char *name);

void suppressNonMax(int width, int height, std::vector<cv::KeyPoint>& keypoints, int radius);

// 
// Save methods
// 

void save_double_as_image(int height, int width, double *array, const char *name);

void save_matches_as_image(int heightl, int widthl, unsigned char *imgl,
                           int heightr, int widthr, unsigned char *imgr,
                           std::vector<MATCH> matches, const char *name);

void save_keypoints_as_image(int height, int width, unsigned char *img, std::vector<KEYPOINT>& points, const char *name);

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

void my_homographies(const std::vector<cv::KeyPoint>& keypoints_left, const std::vector<cv::KeyPoint>& keypoints_right,
                     const std::vector<cv::DMatch>& matches, cv::Mat& Hl, cv::Mat& Hr);

#endif