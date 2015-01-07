#include <iostream>
#include <getopt.h> // getopt_long()

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;


static void usage()
{
    cout << "Usage: ./stereo_match [options] left right" << endl;
    cout << "  options:" << endl;
    cout << "    -h, --help            Show this help message" << endl;
    cout << "    -r, --radius          Block radius for stereo matching. Default: 3" << endl;
    cout << "    -m, --max-disparity   Shrinks the range that will be used" << endl;
    cout << "                          for block matching. Default: 16" << endl;
    cout << "    -t, --target          Name of output file. Default: out.png" << endl;
    cout << "    -f, --filter          Radius of the median filter applied to " << endl;
    cout << "                          the disparity map. If 0, this feature is " << endl;
    cout << "                          disabled. Default: 2" << endl;
}


static bool parsePositionalImage(Mat& image, const string& name, int argc, char const *argv[])
{
    if (optind >= argc) {
        cerr << argv[0] << ": required argument: '" << name << "'" << endl;
        usage();

        return false;
    } else {
        image = imread(argv[optind++], CV_LOAD_IMAGE_GRAYSCALE);

        if (image.empty()) {
            cerr << "Error: Cannot read '" << argv[optind] << "'" << endl;

            return false;
        }
    }

    return true;
}


static float matchSSD(const int radius, const Mat& left, const Mat& right, const Point2i center,
                      const int max_disparity, float* best_match, int* disparity)
{
    // we do not want to cast rapidly from int to float, because the SSD is an
    // integer
    int min_ssd = INT_MAX;

    int start = -max_disparity;
    int end   =  max_disparity;
    
    if (center.x + start < 0) {
        start = -(center.x - radius);
    }
    if (center.x + end > right.cols) {
        end = right.cols - center.x;
    }

    // cout << "center.y = " << center.y << endl;
    // cout << "start    = " << start << endl;
    // cout << "center.y - start = " << (center.y - start) << endl;
    // cout << "end = " << end << endl;

    // Mat patch  = Mat::zeros(2 * radius + 1, 2 * radius + 1, CV_8UC1);
    // Mat search = Mat::zeros(2 * radius + 1, 2 * radius + 1, CV_8UC1);

    for (int col_offset = start; col_offset < end; col_offset += 1) {
        int ssd = 0;

        // walk through the patch
        for (int prow = -radius; prow <= radius; prow++) {
            for (int pcol = -radius; pcol <= radius; pcol++) {

                // patch.at<uchar>(prow + radius, pcol + radius)  = left.at<uchar>(center.y + prow, center.x + pcol);
                // search.at<uchar>(prow + radius, pcol + radius) = right.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                // grayscale images => uchar
                // patch - image
                int diff =    left.at<uchar>(center.y + prow, center.x + pcol)
                           - right.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                ssd  += diff * diff;
            }
        }


        // if (center.y > 200 && center.x > 120) {
        //     imshow("patch", patch);
        //     imshow("search", search);
        //     waitKey(0);

        //     cout << col_offset << ": " << ssd << endl;
        // }


        if (ssd < min_ssd) {
            min_ssd = ssd;
            *disparity = -col_offset;
        }

    }

    // if (center.y > 150) {

    //     Mat canvas;
    //     left.copyTo(canvas);

    //     for (int row = -radius; row < radius; row++) {
    //         for (int col = -radius; col < radius; col++) {
    //             // cout << row << ", " << col << endl;
    //             // canvas.at<uchar>(center.y + row, center.x + col) = left.at<uchar>(center.y + row, center.x + col);
    //             // canvas.at<uchar>(center.y + row, center.x + col) = 0;
    //             // canvas.at<uchar>(center.y + row, center.x + col) = right.at<uchar>(center.y + row, center.x + col);
    //             canvas.at<uchar>(center.y + row, center.x + col) = right.at<uchar>(center.y + row, center.x + col - (int) *disparity);
    //         }
    //     }
    //     imshow("canvas", canvas);
    //     waitKey(10);

    // }

    // cout << "disparity = " << *disparity << endl;

    // *best_match = (float) best_ssd;
}


static void blockMatch(const Mat& left, const Mat& right, Mat& disparity,
                       const int radius, const int max_disparity)
{
    disparity = Mat::zeros(left.size(), DataType<int>::type);

    // walk through the left image
    for (int lrow = radius; lrow < left.rows - radius; lrow++) {

        // cout << "Row " << lrow << endl;
        cout << "." << flush;

        for (int lcol = radius; lcol < left.cols - radius; lcol++) {

            // cout << lrow << ", " << lcol << endl;

            int shift = 0;
            // float best_match = 0;
            float match = 0;

            matchSSD(radius, left, right, Point2i(lcol, lrow), max_disparity, &match, &shift);

            // Mat patch (left,
            //            Range(lrow - radius, lrow + radius),
            //            Range(lcol - radius, lcol + radius));

            // Mat search (right,
            //             Range(lrow - radius, lrow + radius),
            //             Range(0, right.cols));

            // Mat result;

            // matchTemplate(search, patch, result, CV_TM_SQDIFF);



            // double min_val;
            // double max_val;
            // Point min_loc;

            // minMaxLoc(result, &min_val, &max_val, &min_loc);


            // // Mat canvas;
            // // search.copyTo(canvas);

            // // cv::rectangle(
            // //     canvas, 
            // //     min_loc, 
            // //     Point(min_loc.x + patch.cols, min_loc.y + patch.rows), 
            // //     CV_RGB(0,255,0),
            // //     2
            // // );
            // // imshow("search", canvas);
            // // imshow("patch", patch);
            // // waitKey(0);


            // min_loc.y += lrow;

            // shift = min_loc.x - lcol;

            // cout << "min_loc " << min_loc << endl;
            // cout << "cnt_loc [" << lcol << ", " << lrow << "]" << endl;

            // for (int row = min_loc; row < patch.rows; row++) {
            //     for (int col = min_loc; col < patch.cols; col++) {
            //         canvas.at<uchar>(row, col) = right.at<uchar>(row, col)
            //     }
            // }

            // cout << result << endl;
            // cout << "min = " << min_val << " " << min_loc << endl;
            // waitKey(0);

            // if (match > best_match) {
                // best_match = match;
                disparity.at<int>(lrow, lcol) = shift;                
            // }


        }
    }

    cout << endl;
}


/**
 * Normalizes the disparoty map to an grayscale image [0, 255]
 */
void normDisp(Mat& disparity, Mat& normalized)
{
    double minval;
    double maxval;

    // initialize matrix
    normalized = Mat::zeros(disparity.size(), CV_8UC1);

    // search minimum and maximum of the disparoty map 
    minMaxLoc(disparity, &minval, &maxval);

    // cout << "min: " << minval << ", maxval: " << maxval << endl;

    // compute normalization scaling factor to get all values into
    // range [0, 255]
    float norm = 255.0 / (maxval - minval);

    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            normalized.at<uchar>(row, col) =  norm * (disparity.at<int>(row, col) - minval);
        }
    }
}


int main(int argc, char const *argv[])
{
    Mat img_left;
    Mat img_right;
    Mat disparity;

    int radius = 3;
    int max_disparity = 32;
    int median_radius = 2;
    string target = "out.png";


    const struct option long_options[] = {
        { "help",           no_argument,       0, 'h' },
        { "radius",         required_argument, 0, 'r' },
        { "target",         required_argument, 0, 't' },
        { "max-disparity",  required_argument, 0, 'm' },
        { "filter",         required_argument, 0, 'f' },
        0 // end of parameter list
    };

    // parse command line options
    while (true) {
        int index = -1;

        int result = getopt_long(argc, (char **) argv, "hr:t:m:f:", long_options, &index);

        // end of parameter list
        if (result == -1) {
            break;
        }

        switch (result) {
            case 'h':
                usage();
                return 0;

            case 'r':
                radius = atoi(optarg);
                if (radius < 0) {
                    cerr << argv[0] << ": Invalid radius " << optarg << endl;
                    return 1;
                }
                break;

            case 'm':
                max_disparity = atoi(optarg);
                if (max_disparity <= 0) {
                    cerr << argv[0] << ": Invalid maximal disparoty " << optarg << endl;
                    return 1;
                }
                break;

            case 't':
                target = optarg;
                break;

            case 'f':
                median_radius = atoi(optarg);
                if (median_radius < 0) {
                    cerr << argv[0] << ": Invalid median radius " << optarg << endl;
                    return 1;
                }
                break;

            case '?': // missing option
                return 1;

            default: // unknown
                cerr << "unknown parameter: " << optarg << endl;
                break;
        }
    }

    // parse positional arguments
    if (!parsePositionalImage(img_left,  "left",  argc, argv)) { return 1; }
    if (!parsePositionalImage(img_right, "right", argc, argv)) { return 1; }


    cout << "Parameters: " << endl;
    cout << "    radius:        " << radius << endl;
    cout << "    max-disparity: " << max_disparity << endl;
    cout << "    median radius: " << median_radius << endl;
    cout << "    target:        " << target << endl;

    blockMatch(img_left, img_right, disparity, radius, max_disparity);
    // cout << "disparity " << endl << disparity << endl;

    Mat normalized;
    Mat filtered;

    // normalize the result to [ 0, 255 ]
    normDisp(disparity, normalized);

    // you can disable median filtering    
    if (median_radius) {
        // apply median filter
        medianBlur(normalized, filtered, 2 * median_radius + 1);
    } else {
        filtered = normalized;
    }

    // imshow("Disparity", filtered);
    // waitKey(0);

    try {
        imwrite(target, filtered);
    } catch (runtime_error& ex) {
        cerr << "Error: cannot save disparity map to '" << target << "'" << endl;

        return 1;
    }


    return 0;
}