#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <complex>

using namespace cv;
using namespace std;

typedef complex<double> complex2d;

const char* image_names = {"{1| |fruits.jpg|input image name}"};
const Scalar red   = {0, 0, 255};
const Scalar blue  = {255, 0, 0};
const Scalar green = {0, 255, 0};

// trackbar sliders
// 
int max_dist = 75;

// number of Fourier coefficients
int num_coeff = 75;

// pixel selected by user via mouse click
Vec3b selected_pixel = {0, 0, 0};
Mat image;

inline double distance(const Vec3b& pixel)
{
    return pow(selected_pixel[0] - pixel[0], 2) +
           pow(selected_pixel[1] - pixel[1], 2) +
           pow(selected_pixel[2] - pixel[2], 2);
}

static void createBinaryImage(Mat& binary)
{
    binary = Mat::zeros(image.size(), CV_8UC1);

    for (int y = 0; y < image.rows; ++y) {
        for (int x = 0; x < image.cols; ++x) {
            int dist = pow(selected_pixel[0] - image.at<Vec3b>(y,x)[0], 2) +
                       pow(selected_pixel[1] - image.at<Vec3b>(y,x)[1], 2) +
                       pow(selected_pixel[2] - image.at<Vec3b>(y,x)[2], 2);

            if (dist < max_dist) {
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


static void calcFourierCoeff(vector<Point2d>& points, vector<Point_<complex2d>>& coeff)
{
    const int N = points.size();

    // calculate real and imaginary part of each coefficient
    for (int k = 0; k < N; k++) {
        double a = 0; // real part
        double b = 0; // imaginary part

        // calculate a, b for x
        for (int n = 0; n < N; n++) {
            a += points[n].x * cos(-2. * M_PI * k * n / N);
            b += points[n].x * sin(-2. * M_PI * k * n / N);
        }

        coeff[k].x = complex2d(a, b);

        a = 0;
        b = 0;
        // calculate a, b for y
        for (int n = 0; n < N; n++) {
            a += points[n].y * cos(-2. * M_PI * k * n / N);
            b += points[n].y * sin(-2. * M_PI * k * n / N);
        }

        coeff[k].y = complex2d(a, b);
    }
}


static void render(int, void*)
{
    // Binary image
    Mat binary;

    createBinaryImage(binary);


    // Make a copy of the current grayscale image but expand it to 3 channels.
    // This step is required because findContours manipulates the image and
    // we will need the binary image to paint the largest region red (therefore 3 channels).
    Mat colorized(binary.size(), CV_8UC3);
    cvtColor(binary, colorized, CV_GRAY2RGB);
    
    vector<Vec4i> hierarchy;
    vector<vector<Point> > contours;
    int counter_index = findLargestArea(binary, contours, hierarchy);

    if (counter_index >= 0) {
        // Draw largest contour in red to the colorized
        drawContours(colorized, contours, counter_index, red, CV_FILLED, 8, hierarchy);

        // for (int i = 0; i < contours[counter_index].size(); i++) {
        //     colorized.at<Vec3b>(contours[counter_index][i].y, contours[counter_index][i].x) = {0, 255, 0};
        // }

        const int counter_length = contours[counter_index].size();

        vector<Point2d> equidist_points;
        findEquidistantPoints(contours[counter_index], equidist_points, num_coeff);

        // draw the equidistant points on the contour green
        for (int i = 0; i < equidist_points.size(); i++) {
            circle(colorized, equidist_points[i], 2, green, -1);
        }
    }


    // Display result
    imshow("Shape", colorized);
}

static void onMouse(int event, int x, int y, int, void*)
{
    // Only listen on left button clicks
    if (event != EVENT_LBUTTONUP) {
        return;
    }

    // Do not track clicks outside the image
    if (x < 0 || y < 0 || x >= image.cols || y >= image.rows) {
        return;
    }

    selected_pixel = image.at<Vec3b>(y,x);

    render(0,0);
}

int main(int argc, char const *argv[])
{
    CommandLineParser parser(argc, argv, image_names);
    string filename = parser.get<string>("1");
    image = imread(filename, CV_LOAD_IMAGE_COLOR);

    if(image.empty()) {
        cout << "Cannot read image file:" << filename << endl;

        return -1;
    }

    // Create a new windows
    namedWindow("Shape", 1);
    namedWindow("Input", 1);

    // Use a lambda expression to calculate the square maximal distance,
    // because we do not use the square root for better performance.
    // After that, call render directly
    createTrackbar("color dist", "Shape", &max_dist,  255, [] (int, void*) { max_dist *= max_dist; render(0,0); });
    createTrackbar("coeff",      "Shape", &num_coeff, 200, render);

    //set the callback function for any mouse event
    setMouseCallback("Input", onMouse, NULL);

    // Display image
    imshow("Input", image);
    render(0,0);

    // Wait for ESC
    cout << "Press ESC to exit ..." << endl;
    while (true) {
        // Wait for a key stroke indefinitly
        if ((uchar) waitKey(0) == 27) {
            break;
        }
    }

    return 0;
}
