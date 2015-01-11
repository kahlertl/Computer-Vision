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
struct histogram_t {
    int r[256];
    int g[256];
    int b[256];
} histogram;


/**
 * This function calculates the median of a single historgram
 * by summing all values from zero and stop if the sum reaches
 * the middle if the histogram width.
 *
 * The current index in the historgram is the median.
 */
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
    memset(histogram.r, 0, sizeof(histogram.r));
    memset(histogram.g, 0, sizeof(histogram.g));
    memset(histogram.b, 0, sizeof(histogram.b));

    // init histogram
    for (int row = 0; row < window_size; row++) {
        for (int col = 0; col < window_size; col++) {
            // It was tested if it is faster to store the Vec3b pixel and
            // access its components, but this approach is slower than this
            // one.
            histogram.b[image.at<Vec3b>(row,col)[0]]++;
            histogram.g[image.at<Vec3b>(row,col)[1]]++;
            histogram.r[image.at<Vec3b>(row,col)[2]]++;
        }
    }

    int row = radius;

    /*
     * We move in a snake like shape through the image. With that approach we
     * do not need to initialize the historgram for each row. 
     */
    while (true) {
        filtered_image.at<Vec3b>(row,radius)[0] = histo_median(histogram.b);
        filtered_image.at<Vec3b>(row,radius)[1] = histo_median(histogram.g);
        filtered_image.at<Vec3b>(row,radius)[2] = histo_median(histogram.r);

        // move right
        for (int col = radius + 1; col < image.cols - radius; col++) {
            // remove left column
            for (int i = 0; i < window_size; i++) {
                histogram.b[image.at<Vec3b>(row - radius + i, col - radius - 1)[0]]--;
                histogram.g[image.at<Vec3b>(row - radius + i, col - radius - 1)[1]]--;
                histogram.r[image.at<Vec3b>(row - radius + i, col - radius - 1)[2]]--;
            }

            // add right column
            for (int i = 0; i < window_size; i++) {
                histogram.b[image.at<Vec3b>(row - radius + i, col + radius)[0]]++;
                histogram.g[image.at<Vec3b>(row - radius + i, col + radius)[1]]++;
                histogram.r[image.at<Vec3b>(row - radius + i, col + radius)[2]]++;
            }

            // calculate median for each channel
            filtered_image.at<Vec3b>(row,col)[0] = histo_median(histogram.b);
            filtered_image.at<Vec3b>(row,col)[1] = histo_median(histogram.g);
            filtered_image.at<Vec3b>(row,col)[2] = histo_median(histogram.r);
        }

        // right edge. move down
        row++;

        // check if we have reached bottom row
        if (row >= image.rows - radius) {
            break;
        }

        // remove top row
        for (int col = image.cols - window_size; col < image.cols; col++) {
            histogram.b[image.at<Vec3b>(row - radius - 1, col)[0]]--;
            histogram.g[image.at<Vec3b>(row - radius - 1, col)[1]]--;
            histogram.r[image.at<Vec3b>(row - radius - 1, col)[2]]--;
        }

        // add bottom row
        for (int col = image.cols - window_size; col < image.cols; col++) {
            histogram.b[image.at<Vec3b>(row + radius, col)[0]]++;
            histogram.g[image.at<Vec3b>(row + radius, col)[1]]++;
            histogram.r[image.at<Vec3b>(row + radius, col)[2]]++;
        }

        filtered_image.at<Vec3b>(row,image.cols - radius - 1)[0] = histo_median(histogram.b);
        filtered_image.at<Vec3b>(row,image.cols - radius - 1)[1] = histo_median(histogram.g);
        filtered_image.at<Vec3b>(row,image.cols - radius - 1)[2] = histo_median(histogram.r);

        // move left
        for (int col = image.cols - radius - 2; col >= radius; col--) {
            // remove right column
            for (int i = 0; i < window_size; i++) {
                histogram.b[image.at<Vec3b>(row - radius + i, col + radius + 1)[0]]--;
                histogram.g[image.at<Vec3b>(row - radius + i, col + radius + 1)[1]]--;
                histogram.r[image.at<Vec3b>(row - radius + i, col + radius + 1)[2]]--;
            }

            // add left column
            for (int i = 0; i < window_size; i++) {
                histogram.b[image.at<Vec3b>(row - radius + i, col - radius)[0]]++;
                histogram.g[image.at<Vec3b>(row - radius + i, col - radius)[1]]++;
                histogram.r[image.at<Vec3b>(row - radius + i, col - radius)[2]]++;
            }

            filtered_image.at<Vec3b>(row,col)[0] = histo_median(histogram.b);
            filtered_image.at<Vec3b>(row,col)[1] = histo_median(histogram.g);
            filtered_image.at<Vec3b>(row,col)[2] = histo_median(histogram.r);
        }

        // left edge. move down
        row++;

        // check if we have reached bottom row
        if (row >= image.rows - radius) {
            break;
        }

        // remove top row
        for (int col = 0; col < window_size; col++) {
            histogram.b[image.at<Vec3b>(row - radius - 1, col)[0]]--;
            histogram.g[image.at<Vec3b>(row - radius - 1, col)[1]]--;
            histogram.r[image.at<Vec3b>(row - radius - 1, col)[2]]--;
        }

        // add bottom row
        for (int col = 0; col < window_size; col++) {
            histogram.b[image.at<Vec3b>(row + radius, col)[0]]++;
            histogram.g[image.at<Vec3b>(row + radius, col)[1]]++;
            histogram.r[image.at<Vec3b>(row + radius, col)[2]]++;
        }

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

    // parse command line options
    while (true) {
        int index = -1;

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
                cerr << "unknown parameter: " << optarg << endl;
                break;
        }
    }

    // parse arguments
    if (optind != argc - 1) {
        cerr << argv[0] << ": required argument: 'image'" << endl;
        print_help();

        return 1;
    } else {
        image = imread(argv[optind], CV_LOAD_IMAGE_COLOR);

        if (image.empty()) {
            cerr << "Error: Cannot read '" << argv[optind] << "'" << endl;

            return 1;
        }

        // init filtered image
        filtered_image = Mat::zeros(image.size(), image.type());
    }

    // decide if we run in interactive mode or create an output file
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