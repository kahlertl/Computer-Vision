#include <iostream>
#include <getopt.h> // getopt_long()

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

// function pointer to a function that can be used for block matching
typedef float(*match_t)(const int, const Mat&, const Mat&, const Point2i, const int, float*, int*, bool);


static void usage()
{
    cout << "Usage: ./stereo_match [options] left right" << endl;
    cout << "  options:" << endl;
    cout << "    -h, --help            Show this help message" << endl;
    cout << "    -r, --radius          Block radius for stereo matching. Default: 3" << endl;
    cout << "    -d, --max-disparity   Shrinks the range that will be used" << endl;
    cout << "                          for block matching. Default: 16" << endl;
    cout << "    -t, --target          Name of output file. Default: out.png" << endl;
    cout << "    -m, --median          Radius of the median filter applied to " << endl;
    cout << "                          the disparity map. If 0, this feature is " << endl;
    cout << "                          disabled. Default: 2" << endl;
    cout << "    -g, --ground-truth    Optimal disparity image. This activates the" << endl;
    cout << "                          search for the optimal block size for each pixel." << endl;
    cout << "    -c, --correlation     Method for computing correlation. There are:" << endl;
    cout << "                              ssd  sum of square differences" << endl;
    cout << "                              sad  sum of absolute differences" << endl;
    cout << "                              ccr  cross correlation" << endl;
    cout << "                          Default: sad" << endl;
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


static float matchSSD(const int radius, const Mat& prev, const Mat& next, const Point2i center,
                      const int max_disparity, float* best_match, int* disparity, bool inverse)
{
    // we do not want to cast rapidly from int to float, because the SSD is an
    // integer
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
    if (center.x + end > next.cols) {
        end = next.cols - center.x;
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

                // patch.at<uchar>(prow + radius, pcol + radius)  = prev.at<uchar>(center.y + prow, center.x + pcol);
                // search.at<uchar>(prow + radius, pcol + radius) = next.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                // grayscale images => uchar
                // patch - image
                int diff =    prev.at<uchar>(center.y + prow, center.x + pcol)
                           - next.at<uchar>(center.y + prow, center.x + pcol + col_offset);

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
            *disparity = abs(col_offset);
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


static float matchSAD(const int radius, const Mat& prev, const Mat& next, const Point2i center,
                      const int max_disparity, float* best_match, int* disparity, bool inverse)
{
    // we do not want to cast rapidly from int to float, because the SSD is an
    // integer
    int min_sad = INT_MAX;

    // just looking into one direction
    int start = -max_disparity;
    int end   = 0;
    
    // inverse match (from right image to left one)    
    if (max_disparity < 0) {
        start = 0;
        end = max_disparity;
    }
    
    if (center.x + start < 0) {
        start = -(center.x - radius);
    }
    if (center.x + end > next.cols) {
        end = next.cols - center.x;
    }

    for (int col_offset = start; col_offset < end; col_offset += 1) {
        int sad = 0;

        // walk through the patch
        for (int prow = -radius; prow <= radius; prow++) {
            for (int pcol = -radius; pcol <= radius; pcol++) {
                // grayscale images => uchar
                // patch - image
                int diff =   prev.at<uchar>(center.y + prow, center.x + pcol)
                           - next.at<uchar>(center.y + prow, center.x + pcol + col_offset);

                sad  += abs(diff);
            }
        }

        if (sad < min_sad) {
            min_sad = sad;
            *disparity = abs(col_offset);
        }

    }
}

static void blockMatch(const Mat& left, const Mat& right, Mat& disparity,
                       const int radius, const int max_disparity,
                       match_t match_fn, bool inverse = false)
{
    disparity = Mat::zeros(left.size(), CV_8UC1);

    // walk through the left image
    for (int lrow = radius; lrow < left.rows - radius; lrow++) {

        // cout << "Row " << lrow << endl;
        cout << "." << flush;

        for (int lcol = radius; lcol < left.cols - radius; lcol++) {

            // cout << lrow << ", " << lcol << endl;

            int shift = 0;
            // float best_match = 0;
            float result = 0;

            match_fn(radius, left, right, Point2i(lcol, lrow), max_disparity, &result, &shift, inverse);
            // matchSSD(radius, left, right, Point2i(lcol, lrow), max_disparity, &result, &shift);
            // matchSAD(radius, left, right, Point2i(lcol, lrow), max_disparity, &result, &shift);

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

            // if (result > best_match) {
                // best_match = result;
                disparity.at<uchar>(lrow, lcol) = shift;                
            // }


        }
    }

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
                // disparity.at<uchar>(row,col) = 0; // optional
            }
        }
    }

    // search the nearest neighbor that is not occluded
    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            if (occlusion.at<uchar>(row, col)) {
                // search for the first pixel in the column that is not occluded
                for (int i = 0; i < disparity.cols / 2; i++) {
                    // search left
                    if (col - i >= 0 && disparity.at<uchar>(row, col - i) == 0) {
                        disparity.at<uchar>(row, col) = disparity.at<uchar>(row, col - i);
                        break;
                    }
                    // search right
                    if (col + i < disparity.cols && disparity.at<uchar>(row,col + i) == 0) {
                        disparity.at<uchar>(row, col) = disparity.at<uchar>(row, col + i);
                        break;
                    }
                }
            }
        }
    }
}

static void create3DPointCloud(Mat& disparity, Mat& image, Mat& point_cloud) {
    // double minval;
    // double maxval;

    // TODO: commandline parameter for this two
    // cm
    double pixel_size = 0.0014;
    double focal_lenght = 0.784;
    double baseline = 8;

    // for caculation Z-Coordinate of each pixel this factor
    // is the same.
    // depth = baseline * focal_length / disparity * pixel size
    //         --------   ------------               ----------  
    double factor = baseline * focal_lenght / pixel_size;

    // minMaxLoc(disparity, &minval, &maxval);

    // // normalize(gray, disparity, 0, maxval, NORM_MINMAX);

    // fill map
    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            uchar disp = disparity.at<uchar>(row, col);

            if (disp > 0) {
                int z = factor / disp;
                cout << z << endl;

                double alpha = 45 * M_PI / 180;
                Mat rotation = (Mat_<double>(3, 3) <<
                                1,          0,           0,
                                0, cos(alpha), -sin(alpha),
                                0, sin(alpha),  cos(alpha));


                Mat point3D = (Mat_<double>(1,3) <<
                                row + disparity.rows / 2,
                                col + disparity.rows / 2,
                                z );

                Mat result = point3D * rotation;
                Mat back = (Mat_<double>(1,3) <<
                                disparity.rows / 2,
                                disparity.rows / 2,
                                0 );
                result = result - back;
                cout << result << endl;

                point_cloud.at<Vec3b>(row, col) = image.at<Vec3b>(result.at<double>(0)/result.at<double>(2), result.at<double>(1)/result.at<double>(2));
            }

    //         int row_3d = row;
    //         int col_3d = col;

    //         if (disp > 0) {
    //             row_3d = row * maxval / gray.at<uchar>(row, col);
    //             col_3d = col * maxval / gray.at<uchar>(row, col);

    //             if (row_3d >= image.rows) {
    //                 row_3d = row;
    //             }
    //             if (col_3d >= image.cols) {
    //                 col_3d = col;
    //             }

    //             // cout << row << " * " << maxval << " / " << (int) gray.at<uchar>(row, col) << " = " << row_3d << endl;
    //             // cout << col << " * " << maxval << " / " << (int) gray.at<uchar>(row, col) << " = " << col_3d << endl;
    //         }

    //         map.at<Vec3b>(row, col) = image.at<Vec3b>(row_3d, col_3d);


        }
    }
}


/**
 * Normalizes the disparity map to an grayscale image [0, 255].
 * Does not works inplace.
 */
static void normDisp(Mat& disparity, Mat& normalized)
{
    double minval;
    double maxval;

    // initialize matrix if necessary
    normalized = Mat::zeros(disparity.size(), CV_8UC1);

    // search minimum and maximum of the disparity map 
    minMaxLoc(disparity, &minval, &maxval);

    // cout << "min: " << minval << ", maxval: " << maxval << endl;

    // compute normalization scaling factor to get all values into
    // range [0, 255]
    float norm = 255.0 / (maxval - minval);

    for (int row = 0; row < disparity.rows; row++) {
        for (int col = 0; col < disparity.cols; col++) {
            normalized.at<uchar>(row, col) =  norm * (disparity.at<uchar>(row, col) - minval);
        }
    }
}


static void _calcMedianDisp(const Mat& prev, const Mat& next, Mat& disp,
                            const int radius, const int max_disparity, const int median_radius,
                            match_t match_fn)
{
    Mat matched;
    Mat gray;

    blockMatch(prev, next, matched, radius, max_disparity, match_fn);

    // normalize the result to [ 0, 255 ]
    normDisp(matched, gray);

    // you can disable median filtering    
    if (median_radius) {
        // apply median filter
        medianBlur(gray, disp, 2 * median_radius + 1);
    } else {
        disp = gray;
    }
}


int main(int argc, char const *argv[])
{
    Mat img_prev;
    Mat img_next;
    Mat image;         // img_prev, but loaded with colors
    Mat disparity;     // from prev to next
    Mat disparity_n2p; // from next to prev (for left right consistency -- LRC)
    Mat ground_truth;  // optimal disparity map for the image pairs

    int radius = 3;
    int max_disparity = 32;
    int median_radius = 2;
    match_t match_fn = &matchSAD;
    string match_name = "sad";
    string target = "out.png";


    const struct option long_options[] = {
        { "help",           no_argument,       0, 'h' },
        { "radius",         required_argument, 0, 'r' },
        { "target",         required_argument, 0, 't' },
        { "max-disparity",  required_argument, 0, 'd' },
        { "median",         required_argument, 0, 'm' },
        { "ground-truth",   required_argument, 0, 'g' },
        { "correlation",    required_argument, 0, 'c' },
        0 // end of parameter list
    };

    // parse command line options
    while (true) {
        int index = -1;

        int result = getopt_long(argc, (char **) argv, "hr:t:d:m:g:c:", long_options, &index);

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

            case 'd':
                max_disparity = atoi(optarg);
                if (max_disparity <= 0) {
                    cerr << argv[0] << ": Invalid maximal disparity " << optarg << endl;
                    return 1;
                }
                break;

            case 't':
                target = optarg;
                break;

            case 'm':
                median_radius = atoi(optarg);
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
                    cerr << argv[0] << ": cross correlation not implemented yet" << endl;
                    return 1;
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
    if (!parsePositionalImage(img_next, CV_LOAD_IMAGE_GRAYSCALE, "frame2", argc, argv)) { return 1; }

    // convert previous image into grayscale
    cvtColor(image, img_prev, CV_BGR2GRAY);

    cout << "Parameters: " << endl;
    cout << "    radius:        " << radius << endl;
    cout << "    match fn:      " << match_name << endl;
    cout << "    max-disparity: " << max_disparity << endl;
    cout << "    median radius: " << median_radius << endl;
    cout << "    target:        " << target << endl;

    blockMatch(img_prev, img_next, disparity,     radius,  max_disparity, match_fn);
    blockMatch(img_next, img_prev, disparity_n2p, radius,  max_disparity, match_fn, true);

    // Left right consistency
    lrcCompensation(disparity, disparity_n2p, max_disparity);

    // you can disable median filtering    
    if (median_radius) {
        // apply median filter
        medianBlur(disparity, disparity, 2 * median_radius + 1);
    } 
    
    Mat map = Mat::zeros(disparity.size(), CV_8UC3);
    // create3DPointCloud(disparity, image, map);

    Mat gray;
    // normalize the result to [ 0, 255 ]
    normDisp(disparity, gray);

    // find optimal block sizes for each pixel if
    // the ground truth for disparity is given
    if (!ground_truth.empty()) {
        cout << "ground truth given" << endl;
        imshow("gt", ground_truth);
    }

    imshow("map", map);
    imshow("gray", gray);
    waitKey(0);

    try {
        imwrite(target, gray);
    } catch (runtime_error& ex) {
        cerr << "Error: cannot save disparity map to '" << target << "'" << endl;

        return 1;
    }


    return 0;
}