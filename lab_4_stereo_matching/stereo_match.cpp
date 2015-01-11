#include <iostream>
#include <getopt.h> // getopt_long()

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

// function pointer to a function that can be used for block matching
typedef int(*match_t)(const int radius, const Mat& left, const Mat& right, const Point2i center,
                      const int max_disparity, bool inverse);


static void usage()
{
    cout << "Usage: ./stereo_match [options] left right" << endl;
    cout << "  options:" << endl;
    cout << "    -h, --help            Show this help message" << endl;
    cout << "    -r, --radius          Block radius for stereo matching. Default: 2" << endl;
    cout << "    -d, --max-disparity   Shrinks the range that will be used" << endl;
    cout << "                          for block matching. Default: 20" << endl;
    cout << "    -t, --target          Name of output file. Default: disparity.png" << endl;
    cout << "    -m, --median          Radius of the median filter applied to " << endl;
    cout << "                          the disparity map. If 0, this feature is " << endl;
    cout << "                          disabled. Default: 2" << endl;
    cout << "    -g, --ground-truth    Optimal disparity image. This activates the" << endl;
    cout << "                          search for the optimal block size for each pixel." << endl;
    cout << "                          The radius parameter will be used for the step" << endl;
    cout << "                          range [0, step]. For each element in the interval" << endl;
    cout << "                          there will be a match performed with radius = 2^step" << endl;
    cout << "                             2^step" << endl;
    cout << "    -c, --correlation     Method for computing correlation. There are:" << endl;
    cout << "                              ssd  sum of square differences" << endl;
    cout << "                              sad  sum of absolute differences" << endl;
    cout << "                              ccr  cross correlation" << endl;
    cout << "                          Default: sad" << endl;
    cout << "    -l, --lrc-threshold   Maximal distance in left-right-consistency check." <<  endl;
    cout << "                          If left and right flow must differ more than this" << endl;
    cout << "                          parameter, the region is considered as occluded." << endl;
    cout << "                          If negative, LRC will be disabled. Default: 3" << endl;
}


static bool parsePositionalImage(Mat& image, const int channels, const string& name, int argc, char const *argv[])
{
    if (optind >= argc) {
        cerr << argv[0] << ": required argument: '" << name << "'" << endl;
        usage();

        return false;
    } else {
        image = imread(argv[optind++], channels);

        if (image.empty()) {
            cerr << "Error: Cannot read '" << argv[optind] << "'" << endl;

            return false;
        }
    }

    return true;
}


static int matchSSD(const int radius, const Mat& left, const Mat& right, const Point2i center,
                      const int max_disparity, bool inverse)
{
    int disparity = 0;
    int min_ssd = INT_MAX;

    // just looking into one direction
    // standard: patch from left image could be
    // found left from its position in right image
    int start = -max_disparity;
    int end   = 0;
    
    // inverse match (from right image to left one)    
    if (inverse) {
        start = 0;
        end = max_disparity;
    }
    
    if (center.x + start < 0) {
        start = -(center.x - radius);
    }
    if (center.x + end > right.cols) {
        end = right.cols - center.x;
    }

    for (int col_offset = start; col_offset < end; col_offset += 1) {
        int ssd = 0;

        // walk through the patch
        for (int prow = -radius; prow <= radius; prow++) {
            for (int pcol = -radius; pcol <= radius; pcol++) {
                // grayscale images => uchar
                // patch - image
                int diff =   left.at<uchar>(center.y + prow, center.x + pcol)
                           - right.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                ssd  += diff * diff;
            }
        }

        if (ssd < min_ssd) {
            min_ssd = ssd;
            disparity = abs(col_offset);
        }
    }

    return disparity;
}


static int matchSAD(const int radius, const Mat& left, const Mat& right, const Point2i center,
                      const int max_disparity, bool inverse)
{
    int disparity = 0;
    int min_sad = INT_MAX;

    // just looking into one direction
    int start = -max_disparity;
    int end   = 0;
    
    // inverse match (from right image to left one)    
    if (inverse) {
        start = 0;
        end = max_disparity;
    }
    
    if (center.x + start < 0) {
        start = -(center.x - radius);
    }
    if (center.x + end > right.cols) {
        end = right.cols - center.x;
    }

    for (int col_offset = start; col_offset < end; col_offset += 1) {
        int sad = 0;

        // walk through the patch
        for (int prow = -radius; prow <= radius; prow++) {
            for (int pcol = -radius; pcol <= radius; pcol++) {
                // grayscale images => uchar
                // patch - image
                int diff =   left.at<uchar>(center.y + prow, center.x + pcol)
                           - right.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                sad  += abs(diff);
            }
        }

        if (sad < min_sad) {
            min_sad = sad;
            disparity = abs(col_offset);
        }
    }

    return disparity;
}


static int matchCCR(const int radius, const Mat& left, const Mat& right, const Point2i center,
                      const int max_disparity, bool inverse)
{
    const int patch_size = 2 * radius + 1;    
    int search_width     = max_disparity + 2 * radius;
    
    int disparity = 0;

    // top left corner of the search area
    Point2i search_corner;
    search_corner.y = center.y - radius;

    // should we search to the right side?
    if (!inverse) {
        search_corner.x = center.x - search_width + radius;

        if (search_corner.x < 0) {
            search_width += search_corner.x;
            search_corner.x = 0;
        }
    } else {
        search_corner.x = center.x -radius;

        if (search_corner.x + search_width >= right.cols) {
            search_width = (right.cols - 1) - search_corner.x;
        }
    }

    // cut off the patch and search area from the image
    Mat patch       = left(Rect(center.x - radius, center.y - radius, patch_size,   patch_size));
    Mat search_area = right(Rect(search_corner.x,   search_corner.y,   search_width, patch_size));

    // perform normed cross correlation matching
    Mat result;
    matchTemplate(search_area, patch, result, CV_TM_CCORR_NORMED);

    // search for the best match
    float max_ccr = -INFINITY;

    for (int col = 0; col < result.cols; col++) {
        if (result.at<float>(0, col) > max_ccr) {
            max_ccr = result.at<float>(0, col);
            disparity = col;
        }
    }

    // if we have searched on the left side, we have to 
    // 
    //    0   1   2   3   4   5 
    //  -------------------------
    //  |   |   |   | x |   |   |
    //  -------------------------
    //                    ^
    //                    |
    //                  center.x
    // 
    // we have to cout the cols from the right to the
    // maximum, in this example: 5 - 3 = 2
    // 
    if (!inverse) {
        disparity = result.cols - disparity;
    }

    return disparity;
}


static void blockMatch(const Mat& left, const Mat& right, Mat& disparity,
                       const int radius, const int max_disparity,
                       match_t match_fn, bool inverse = false)
{
    disparity = Mat::zeros(left.size(), CV_8UC1);

    // walk through the left image
    for (int lrow = radius; lrow < left.rows - radius; lrow++) {
        // shows progress
        cout << "." << flush;

        for (int lcol = radius; lcol < left.cols - radius; lcol++) {
            disparity.at<uchar>(lrow, lcol) = match_fn(radius, left, right, Point2i(lcol, lrow),
                                                       max_disparity, inverse);;                
        }
    }

    // final line break for the progess dot bar
    cout << endl;
}


/**
 * Left right consistency compensation
 */
static void lrcCompensation(Mat& disparity, const Mat& disparity_revert, const uint max_diff)
{
    Mat occlusion = Mat::zeros(disparity.size(), disparity.type());

    // detect occluded regions
    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            // diff.at<uchar>(row,col) = abs(disparity.at<uchar>(row,col) - disparity_revert.at<uchar>(row,col));
            uint diff = abs(disparity.at<uchar>(row,col) - disparity_revert.at<uchar>(row,col));

            if (diff > max_diff) {
                occlusion.at<uchar>(row,col) = 1;

                // paint occluded areas black
                // not needed, but for debugging very useful
                disparity.at<uchar>(row,col) = 0; // optional
            }
        }
    }

    // search the nearest neighbor that is not occluded
    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            if (occlusion.at<uchar>(row, col) == 1) {
                // search for the first pixel in the column that is not occluded
                for (int i = 0; i < disparity.cols / 2; i++) {
                    // search left
                    if (col - i >= 0 && occlusion.at<uchar>(row, col - i) == 0) {
                        disparity.at<uchar>(row, col) = disparity.at<uchar>(row, col - i);
                        break;
                    }
                    // search right
                    if (col + i < disparity.cols && occlusion.at<uchar>(row,col + i) == 0) {
                        disparity.at<uchar>(row, col) = disparity.at<uchar>(row, col + i);
                        break;
                    }
                }
            }
        }
    }
}


static void stereoMatch(const Mat& left, const Mat& right, Mat& disparity,
                                const int radius, const int max_disparity, const int median_radius,
                                match_t match_fn, int lrc = -1) 
{
    Mat disparity_n2p; // from right to left (for left right consistency -- LRC)

    blockMatch(left, right, disparity, radius, max_disparity, match_fn);

    // Left right consistency
    if (lrc > 0) {
        // match in the other direction
        blockMatch(right, left, disparity_n2p, radius, max_disparity, match_fn, true);

        // compute occluded regions and in paint them with the nearest neighbor
        // in the column that is consistent
        lrcCompensation(disparity, disparity_n2p, lrc);
    }

    // you can disable median filtering    
    if (median_radius) {
        // apply median filter
        medianBlur(disparity, disparity, 2 * median_radius + 1);
    } 
}


int main(int argc, char const *argv[])
{
    Mat left;
    Mat right;
    Mat image;         // left, but loaded with colors
    Mat disparity;     // from left to right
    Mat ground_truth;  // optimal disparity map for the image pairs

    // parameters
    int radius         = 2;
    int max_disparity  = 20;
    int median_radius  = 2;
    match_t match_fn   = &matchSAD;
    string match_name  = "sad";
    string target      = "disparity.png";
    int lrc_threshold  = 3;

    const struct option long_options[] = {
        { "help",           no_argument,       0, 'h' },
        { "radius",         required_argument, 0, 'r' },
        { "target",         required_argument, 0, 't' },
        { "max-disparity",  required_argument, 0, 'd' },
        { "median",         required_argument, 0, 'm' },
        { "ground-truth",   required_argument, 0, 'g' },
        { "correlation",    required_argument, 0, 'c' },
        { "lrc-threshold",  required_argument, 0, 'l' },
        0 // end of parameter list
    };

    // parse command line options
    while (true) {
        int index = -1;

        int result = getopt_long(argc, (char **) argv, "hr:t:d:m:g:c:l:", long_options, &index);

        // end of parameter list
        if (result == -1) {
            break;
        }

        switch (result) {
            case 'h':
                usage();
                return 0;

            case 'l':
                lrc_threshold = stoi(string(optarg));
                break;

            case 'r':
                radius = stoi(string(optarg));
                if (radius < 0) {
                    cerr << argv[0] << ": Invalid radius " << optarg << endl;
                    return 1;
                }
                break;

            case 'd':
                max_disparity = stoi(string(optarg));
                if (max_disparity <= 0) {
                    cerr << argv[0] << ": Invalid maximal disparity " << optarg << endl;
                    return 1;
                }
                break;

            case 't':
                target = optarg;
                break;

            case 'm':
                median_radius = stoi(string(optarg));
                if (median_radius < 0) {
                    cerr << argv[0] << ": Invalid median radius " << optarg << endl;
                    return 1;
                }
                break;

            case 'g':
                ground_truth = imread(optarg, CV_LOAD_IMAGE_GRAYSCALE);
                break;

            case 'c':
                match_name = string(optarg);

                if (match_name == "ssd") {
                    match_fn = &matchSSD;
                } else if (match_name == "sad") {
                    match_fn = &matchSAD;
                } else if (match_name == "ccr") {
                    match_fn = &matchCCR;
                } else {
                    cerr << argv[0] << ": Invalid correlation method '" << optarg << "'" << endl;
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
    if (!parsePositionalImage(image,    CV_LOAD_IMAGE_COLOR,     "frame1", argc, argv)) { return 1; }
    if (!parsePositionalImage(right, CV_LOAD_IMAGE_GRAYSCALE, "frame2", argc, argv)) { return 1; }

    // convert previous image into grayscale
    cvtColor(image, left, CV_BGR2GRAY);

    cout << "Parameters: " << endl;
    cout << "    radius:        " << radius << endl;
    cout << "    match fn:      " << match_name << endl;
    cout << "    max-disparity: " << max_disparity << endl;
    cout << "    median radius: " << median_radius << endl;
    cout << "    ground truth:  " << ((ground_truth.empty()) ? "false" : "true") << endl;
    cout << "    LRC threshold: " << lrc_threshold << endl;
    cout << "    target:        " << target << endl;

    // find optimal block sizes for each pixel if
    // the ground truth for disparity is given
    if (!ground_truth.empty()) {
        normalize(ground_truth, ground_truth, 0, 255, NORM_MINMAX);

        // imshow("GT", ground_truth);
        // waitKey(0);

        uint steps = radius;

        // try different block sizes 
        vector<Mat> disparities(steps);
        for (int i = 0; i < steps; i++) {
            // variable radius
            int var_radius = pow(2, i);

            cout << "block size: " << (2 * var_radius + 1) << endl;

            stereoMatch(left, right, disparities[i],
                        // parameters
                        var_radius, max_disparity, median_radius, match_fn, lrc_threshold);

            // normalize result to [0, 255]
            normalize(disparities[i], disparities[i], 0, 255, NORM_MINMAX);
        }

        // compare different disparities to ground truth and save block size
        Mat opt_block_size = Mat(left.size(), left.type());
            disparity      = Mat(left.size(), left.type());

        for (int row = 0; row < left.rows; row++) {
            for (int col = 0; col < left.cols; col++) {
                int smallest_diff = INT_MAX;
                for (int i = 0; i < steps; i++) {
                    int diff = ground_truth.at<uchar>(row, col) - disparities[i].at<uchar>(row, col);
                    if (abs(diff) < smallest_diff) {
                        smallest_diff = diff;
                        opt_block_size.at<uchar>(row, col) = i;
                        disparity.at<uchar>(row, col) = disparities[i].at<uchar>(row, col);
                    }
                }
            }
        }

        normalize(opt_block_size, opt_block_size, 0, 255, NORM_MINMAX);

        // display block sizes
        imshow("Optimal block size for each pixel", opt_block_size);

        // wait for ESC key
        while (true) {
            if ((uchar) waitKey(0) == 27) {
                break;
            }
        }
        // write optimal block size to file
        // imwrite("opt-block-size.png", opt_block_size);

    } else {
        stereoMatch(left, right, disparity,
                    // parameters
                    radius, max_disparity, median_radius, match_fn, lrc_threshold);
    }
     
    // normalize the result to [ 0, 255 ]
    normalize(disparity, disparity, 0, 255, NORM_MINMAX);

    try {
        imwrite(target, disparity);
    } catch (runtime_error& ex) {
        cerr << "Error: cannot save disparity map to '" << target << "'" << endl;

        return 1;
    }

    return 0;
}