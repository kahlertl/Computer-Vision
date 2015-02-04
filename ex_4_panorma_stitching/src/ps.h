#ifndef __PANST
#define __PANST

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

// save all intermediate results as images
#define SAVE_ALL

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

vector<KEYPOINT> harris(int height, int width, unsigned char *img,
                        int wsize_sum, int wsize_loc, const char *name);
vector<MATCH> matching(int heightl, int widthl, unsigned char *imgl,
                       int heightr, int widthr, unsigned char *imgr,
                       vector<KEYPOINT>pointsl, vector<KEYPOINT>pointsr, int wsize);
void mean_filter(int height, int width, double *a, int wsize);
vector<KEYPOINT> local_maxima(int height, int width, double *e, int wsize, const char *name);
void save_double_as_image(int height, int width, double *array, const char *name);
void save_matches_as_image(int heightl, int widthl, unsigned char *imgl,
                           int heightr, int widthr, unsigned char *imgr,
                           vector<MATCH> matches, const char *name);
void save_keypoints_as_image(int height, int width, unsigned char *img,
                             vector<KEYPOINT> points, const char *name);
inline void ht(double x0, double y0, Mat &H, double *x, double *y)
{
    Mat point1 = (Mat_<double>(3, 1) << x0, y0, 1.);
    Mat point2 = H * point1;
    *x = point2.at<double>(0, 0) / point2.at<double>(2, 0);
    *y = point2.at<double>(1, 0) / point2.at<double>(2, 0);
}
void render(int heightl, int widthl, Mat &imgl,
            int heightr, int widthr, Mat &imgr,
            Mat Hl, Mat Hr, const char *name);
void my_homographies(vector<MATCH> matches, Mat &Hl, Mat &Hr);
#endif