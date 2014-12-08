/**
 * Fast Median Filter
 *
 * Using the ArgvParser from Michael Hanke <michael.hanke@gmail.com>
 * (http://mih.voxindeserto.de/argvparser.html)
 * 
 * @author: Lucas Kahlert <lucas.kahlert@tu-dresden.de>
 */
#include <iostream> // std::cout
#include <sstream>
#include <getopt.h> // getopt_long()
#include "opencv2/highgui/highgui.hpp" // cv:imread, cv::imshow, cv::waitKey

using namespace std;
using namespace cv;

// images
Mat image;
Mat filtered_image;

// filter parameters
int radius = 1;
int window_size;
int window_area;

// histograms
int histogram[256];

static inline int histo_median(const int (&histogram)[256])
{
    int i = 0;

    for (int sum = 0; i < 256; i++) {
        sum += histogram[i];

        if (sum > window_area / 2) {
            break;
        }
    }

    return i;
}

void print_help()
{
    cout << "Usage: ./fast_median_filter [options] image" << endl;
    cout << "  options:" << endl;
    cout << "    -h, --help         Show this help message" << endl;
    cout << "    -r, --radius       Filter radius. Requires an argument. Default: 1" << endl;
    cout << "    -t, --target       Name of output file. If no target is specified," << endl;
    cout << "                       the program will run in 'interactive' mode " << endl;
    cout << "                       displaying an windows with trackbar for the radius" << endl;
}



void huang_median()
{
    // zero the histogram
    memset(histogram, 0, sizeof(histogram));

    // init histogram
    for (int row = 0; row < window_size; row++) {
        for (int col = 0; col < window_size; col++) {
            histogram[image.at<uchar>(row, col)]++;
        }
    }

    int row = radius;

    while (true) {
        filtered_image.at<uchar>(row,radius) = histo_median(histogram);

        // move right
        for (int col = radius + 1; col < image.cols - radius; col++) {
            // remove left column
            // cout << "Remove right column" << (col - radius - 1) << endl;
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col - radius - 1)]--;
            }

            // add right column
            // cout << "Add right column " << (col + radius) << endl;
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col + radius)]++;
            }

            filtered_image.at<uchar>(row,col) = histo_median(histogram);
        }

        // right edge. move down
        row++;

        if (row > image.rows - radius) {
            break;
        }

        // remove top row
        // cout << "Remove right top row " << (row - radius - 1) << ": ";
        for (int col = image.cols - window_size; col < image.cols; col++) {
            // cout << col << " ";
            histogram[image.at<uchar>(row - radius - 1, col)]--;
        }
        // cout << endl;

        // add bottom row
        // cout << "Add right bottom row " << (row + radius) << ": ";
        for (int col = image.cols - window_size; col < image.cols; col++) {
            // cout << col << " ";
            histogram[image.at<uchar>(row + radius, col)]++;
        }
        // cout << endl;

        filtered_image.at<uchar>(row,image.cols - radius - 1) = histo_median(histogram);

        // move left
        for (int col = image.cols - radius - 2; col >= radius; col--) {
            // remove right column
            // cout << "Remove right column" << (col + radius + 1) << endl;
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col + radius + 1)]--;
            }

            // add left column
            // cout << "Add left column " << (col - radius) << endl;
            for (int i = 0; i < window_size; i++) {
                histogram[image.at<uchar>(row - radius + i, col - radius)]++;
            }

            filtered_image.at<uchar>(row,col) = histo_median(histogram);
        }

        // left edge. move down
        row++;

        if (row > image.rows - radius) {
            break;
        }

        // remove top row
        // cout << "Remove left top row " << (row - radius - 1) << ": ";
        for (int col = 0; col < window_size; col++) {
            // cout << col << " ";
            histogram[image.at<uchar>(row - radius - 1, col)]--;
        }
        // cout << endl;

        // add bottom row
        // cout << "Add left bottom row " << (row + radius) << endl;
        for (int col = 0; col < window_size; col++) {
            histogram[image.at<uchar>(row + radius, col)]++;
        }

        // cout << "Pass row" << row << endl;
    }
}

void on_trackbar(int, void*)
{
    window_size = 2 * radius + 1;
    window_area = window_size * window_size;

    huang_median();

    imshow("Median filter", filtered_image);
}


int main(int argc, const char* argv[])
{
    // file name of the filtered image if not in interactive mode
    string target;

    const struct option long_options[] = {
        { "radius",      required_argument, 0, 'r' },
        { "target",      required_argument, 0, 't' },
        { "help",        no_argument,       0, 'h' },
        0 // end of parameter list
    };

    while (true) {
        int index = -1;
        struct option* opt = 0;

        int result = getopt_long(argc, (char **) argv, "h::r:i::t:", long_options, &index);

        // end of parameter list
        if (result == -1) {
            break;
        }

        switch (result) {
            case 'h':
                print_help();
                return 0;

            case 'r':
                radius = atoi(optarg);
                if (radius == 0) {
                    cerr << argv[0] << ": Invalid radius " << optarg << endl;
                    return 1;
                }
                window_size = 2 * radius + 1;
                window_area = window_size * window_size;
                break;

            case 't':
                target = optarg;
                break;

            case '?': // missing option
                return 1;

            default: // unknown
                cout << "unknown parameter: " << optarg << endl;
                break;
        }
    }

    if (optind != argc - 1) {
        cerr << argv[0] << ": required argument: 'image'" << endl;
        print_help();

        return 1;
    } else {
        // image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
        image = imread(argv[optind], CV_LOAD_IMAGE_GRAYSCALE);
        if (image.empty()) {
            cerr << "Error: Cannot read '" << argv[optind] << "'" << endl;

            return 1;
        }

        // init filtered image
        filtered_image = Mat::zeros(image.size(), image.type());
    }


    if (target.empty()) {
        // create interactive scene
        namedWindow("Median filter", 1);
        createTrackbar("Radius", "Median filter", &radius, 50, on_trackbar);

        // initial rendering
        on_trackbar(0, NULL);

        // wait indefinitly on a key stroke
        waitKey(0);
    } else {
        huang_median();

        try {
            imwrite(target, filtered_image);
        } catch (runtime_error& ex) {
            cerr << "Error: saving filtered image to '" << target << "'" << endl;

            return 1;
        }

    }


    return 0;
}