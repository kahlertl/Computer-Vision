#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <complex>

using namespace cv;
using namespace std;

typedef vector<complex<double>> FourierCoeff;

typedef struct Fourier2D {
    FourierCoeff x;
    FourierCoeff y;
} Fourier2D;

// Some color definitions
const Scalar red   = {0, 0, 255};
const Scalar blue  = {255, 0, 0};
const Scalar green = {0, 255, 0};
const Scalar white = {255, 255, 255};

// maximal color distance
int color_threshold = 160;

// number of Fourier coefficients
int num_coeff = 40;

// number of contour points that will be used
// to calculate the coefficients
int num_points = 1024;

// pixel selected by user via mouse click
Vec3b selected_pixel = {227, 252, 255};

// input image
Mat image;


static void createBinaryImage(Mat& binary, int max_color_dist)
{
    binary = Mat::zeros(image.size(), CV_8UC1);

    // Use square maximal distance to eliminate the usage
    // of a square root
    max_color_dist *= max_color_dist;

    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            int dist = pow(selected_pixel[0] - image.at<Vec3b>(y,x)[0], 2) +
                       pow(selected_pixel[1] - image.at<Vec3b>(y,x)[1], 2) +
                       pow(selected_pixel[2] - image.at<Vec3b>(y,x)[2], 2);

            if (dist < max_color_dist) {
                binary.at<uchar>(y,x) = 255;
            }
        }
    }
}

static int findLargestArea(Mat& search, vector<vector<Point> >& contours, vector<Vec4i>& hierarchy)
{
    findContours(search, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

    if (contours.empty()) {
        return -1;
    }

    double max_area = 0;
    // index of the contour with the largest area
    int max_i = 0;
    // index of the current contour we are looking at
    int i = 0;

    // Search as long there as there is a next contour in the top level
    // of the hierarchy
    while (i >= 0) {
        double area = contourArea(contours[i]);
        int child = hierarchy[i][2];

        // Subtract the area of all holes
        while (child >= 0) {
            area -= contourArea(contours[child]);
            child = hierarchy[child][2];
        }

        if (max_area < area) {
            max_area = area;
            max_i = i;
        }

        // Go to next contour in the top hierarchy level
        i = hierarchy[i][0];
    }
    return max_i;
}


/**
 * Euclidian distance between to points
 */
static inline double dst(const Point& a, const Point& b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) +
                (a.y - b.y) * (a.y - b.y));
}


/**
 * Calculates n equidistant points on the given contour. The function uses
 * subpixel precison.
 * 
 * @param contour
 * @param points  result vector of subpixel precison points
 * @param n       number of points
 */
static void findEquidistantPoints(const vector<Point>& contour, vector<Point2d>& points, const int n)
{
    double perimeter = arcLength(contour, true);

    // index of the current contour point
    int j = 0;

    // arcus length of the current contour point
    double arc_len = 0;
    // arcus length of the previous contour point
    double arc_len_prev = 0;


    for (int i = 0; i < n; i++) {
        // arcus length of the next equidistant point
        double arc_len_i = i * perimeter / n;

        // walk along the contour
        while (arc_len_i >= arc_len) {
            // current arc length becomes previous
            arc_len_prev = arc_len;

            // add distance to the next contour point
            arc_len += dst(contour[j], contour[(j + 1) % contour.size()]);

            // update contour index
            j = (j + 1) % contour.size();
        }

        // normalize vector between current and previous contour point and 
        // scale it in that way, that it points from the previous contour
        // point to the current contour point
        double norm_scale = (arc_len_i - arc_len_prev) / (arc_len - arc_len_prev);

        // index of the previous contour point
        int j_prev = (j - 1) % contour.size();

        Point2d point;
        point.x = norm_scale * (contour[j].x - contour[j_prev].x) + contour[j_prev].x;
        point.y = norm_scale * (contour[j].y - contour[j_prev].y) + contour[j_prev].y;

        points.push_back(point);
    }
}


static void calcFourierCoeff(vector<Point2d>& points, Fourier2D& coeff, const int num_coeff)
{
    const int N = points.size();

    // make enough space for all coefficients
    coeff.x.resize(N);
    coeff.y.resize(N);

    // calculate real and imaginary part of each coefficient
    for (int k = 0; k < num_coeff; k++) {
        double a = 0; // real part
        double b = 0; // imaginary part

        // calculate a, b for x
        for (int n = 0; n < N; n++) {
            a += points[n].x * cos(2.0 * M_PI * k * n / N);
            b += points[n].x * sin(2.0 * M_PI * k * n / N);
        }

        a = (a * 2) / N;
        b = (b * 2) / N;

        coeff.x[k] = complex<double>(a, b);

        // reset
        a = 0;
        b = 0;

        // calculate a, b for y
        for (int n = 0; n < N; n++) {
            a += points[n].y * cos(2.0 * M_PI * k * n / N);
            b += points[n].y * sin(2.0 * M_PI * k * n / N);
        }

        a = (a * 2) / N;
        b = (b * 2) / N;

        // cout << "y: a = " << a << ", b = " << b << endl;


        coeff.y[k] = complex<double>(a, b);
    }
}


static double fourierFn(const double x, const vector<complex<double>>& coeffs, const double perimeter)
{
    double f = coeffs[0].real() / 2.0;

    for (int n = 1; n < coeffs.size(); n++) {
        f += coeffs[n].real() * cos(2.0 * M_PI * n * x / perimeter) + 
             coeffs[n].imag() * sin(2.0 * M_PI * n * x / perimeter);
    }

    return f;
}


static void drawFourier(Mat& image, const Fourier2D& coeff, const int perimeter, const Scalar& color)
{
    Point first = Point(fourierFn(0, coeff.x, perimeter),
                        fourierFn(0, coeff.y, perimeter));
    Point prev = first;

    for (int i = 1; i < perimeter; i++) {
        Point point = Point((int) fourierFn(i, coeff.x, perimeter),
                            (int) fourierFn(i, coeff.y, perimeter));

        // draw line
        line(image, prev, point, color, 2, 8);

        prev = point;
    }

    // connect first and last point
    line(image, prev, first, color, 2, 8);
}


/**
 * Handler for all sliders
 */
static void render(int, void*)
{
    // Binary image
    Mat binary;

    // original image with approximated shape
    Mat marked;
    image.copyTo(marked);

    createBinaryImage(binary, color_threshold);

    // sanitize parameters
    // 
    // Minimal point number
    if (num_points < 2) {
        num_points = 2;

        cerr << "Warning! Minimal amount of points is 2" << endl;
    }

    // we need at least the same number of points as number of coefficients
    if (num_points < num_coeff) {
        num_coeff = num_points - 1;

        cerr << "Warning! Cannot have more coefficients than points." << endl;
        cerr << "         Set number of coefficients to number of points." << endl;
    }

    // Make a copy of the current grayscale image but expand it to 3 channels.
    // This step is required because findContours manipulates the image and
    // we will need the binary image to paint the largest region red (therefore 3 channels).
    Mat colorized(binary.size(), CV_8UC3);
    cvtColor(binary, colorized, CV_GRAY2RGB);
    
    vector<Vec4i> hierarchy;
    vector<vector<Point>> contours;
    int counter_index = findLargestArea(binary, contours, hierarchy);

    if (counter_index >= 0) {
        // Draw largest contour in red to the colorized
        drawContours(colorized, contours, counter_index, red, CV_FILLED, 8, hierarchy);

        vector<Point2d> equidist_points;
        findEquidistantPoints(contours[counter_index], equidist_points, num_points);

        // draw the equidistant points on the contour
        for (int i = 0; i < equidist_points.size(); i++) {
            circle(colorized, equidist_points[i], 2, blue, -1);
        }

        Fourier2D coeffs;
        calcFourierCoeff(equidist_points, coeffs, num_coeff);

        drawFourier(marked, coeffs, 100, green);
    }


    // Display result
    imshow("Shape", colorized);
    imshow("Input", marked);
}


/**
 * Mouse click handler
 */
static void onMouse(int event, int x, int y, int, void*)
{
    // only listen on left button clicks
    if (event != EVENT_LBUTTONUP) {
        return;
    }

    // do not track clicks outside the image
    if (x < 0 || y < 0 || x >= image.cols || y >= image.rows) {
        return;
    }

    selected_pixel = image.at<Vec3b>(y,x);

    render(0,0);
}


int main(int argc, char const *argv[])
{
    if (argc < 2) {
        cerr << "Usage: ./shape <image>" << endl;

        return 1;
    }

    string filename = argv[1];
    image = imread(filename, CV_LOAD_IMAGE_COLOR);

    if(image.empty()) {
        cout << "Cannot read image file '" << filename << "'" << endl;

        return 1;
    }

    // create a new windows
    namedWindow("Shape", 1);
    namedWindow("Input", 1);
    namedWindow("Input", 1);

    // sliders
    createTrackbar("color dist", "Shape", &color_threshold,    255, render);
    createTrackbar("coeff",      "Shape", &num_coeff,   200, render);
    createTrackbar("points",     "Shape", &num_points, 2048, render);

    // set the callback function for any mouse event
    setMouseCallback("Input", onMouse, NULL);

    // display image
    imshow("Input", image);
    render(0,0);

    // wait for ESC
    cout << "Press ESC to exit ..." << endl;
    while (true) {
        // Wait for a key stroke indefinitly
        if ((uchar) waitKey(0) == 27) {
            break;
        }
    }

    return 0;
}
