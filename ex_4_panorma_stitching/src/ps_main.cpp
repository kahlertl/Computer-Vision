// standard stuff
#include <stdio.h>
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <time.h>
#include <fstream>

// opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/calib3d/calib3d.hpp"
using namespace cv;

// the header for all functions/macros of this project
#include "ps.h"

int main(int argc, char **argv) {
    if( argc!=3 ) {
        cerr << endl << "Usage: ps <left image> <right image>" << endl << endl;
        exit(0);
    }

    // read images
    Mat imgl = imread(argv[1]);
    if( !(imgl.data) ) {
        cerr << endl << "Can not read " << argv[1] << endl;
        exit(0);
    }
    int heightl=imgl.rows;
    int widthl=imgl.cols;

    Mat imgr = imread(argv[2]);
    if( !(imgr.data) ) {
        cerr << endl << "Can not read " << argv[2] << endl;
        exit(0);
    }
    int heightr=imgr.rows;
    int widthr=imgr.cols;

    // faust regel for window sizes :-)
    int wsize_sum=(int)((widthl+widthr+heightl+heightr)/1300.+0.5);
    if( wsize_sum<1 ) wsize_sum=1;
    int wsize_loc=(int)((widthl+widthr+heightl+heightr)/500.+0.5);
    if( wsize_loc<2 ) wsize_loc=2;
    int wsize_match=(int)((widthl+widthr+heightl+heightr)/90.+0.5);
    if( wsize_match<5 ) wsize_match=5;
    if( wsize_match>40 ) wsize_match=40;

    cerr << endl << "Images loaded. wsize_sum=" << wsize_sum
         << ", wsize_loc=" << wsize_loc
         << ", wsize_match=" << wsize_match;

    // Harris detector
    cerr << endl << "Start Harris detector ...";
    vector<KEYPOINT> pointsl=harris(heightl,widthl,imgl.ptr(0),wsize_sum,wsize_loc,"L");
    cerr << endl << pointsl.size() << " keypoints in the left image";
#ifdef SAVE_ALL
    save_keypoints_as_image(heightl,widthl,imgl.ptr(0),pointsl,"keypointsL.png");
#endif

    vector<KEYPOINT> pointsr=harris(heightr,widthr,imgr.ptr(0),wsize_sum,wsize_loc,"R");
    cerr << endl << pointsr.size() << " keypoints in the right image";
#ifdef SAVE_ALL
    save_keypoints_as_image(heightr,widthr,imgr.ptr(0),pointsr,"keypointsR.png");
#endif

    // Matching
    cerr << endl << "Start matching ...";
    vector<MATCH> matches=matching(heightl,widthl,imgl.ptr(0),heightr,widthr,imgr.ptr(0),pointsl,pointsr,wsize_match);
    cerr << endl << matches.size() << " matching pairs found";
#ifdef SAVE_ALL
    save_matches_as_image(heightl,widthl,imgl.ptr(0),heightr,widthr,imgr.ptr(0),matches,"matches.png");
#endif

    // homography -- OpenCV implementation
    cerr << endl << "Start standard RANSAC ...";
    vector<Point2d> v1;
    vector<Point2d> v2;
    for( unsigned int i=0; i < matches.size(); i++ ) {
        v1.push_back(Point2d(matches[i].xl,matches[i].yl));
        v2.push_back(Point2d(matches[i].xr,matches[i].yr));
    }
    Mat H = findHomography(v1,v2,CV_RANSAC);
    Mat Hi = H.inv();
    cerr << " done";

    // render the output
    Mat imglt=imgr.clone();
    warpPerspective(imgl,imglt,H,imglt.size(),INTER_LINEAR);
    imglt=imglt*0.5+imgr*0.5;
    imwrite("warpedL.png",imglt);
    Mat imgrt=imgl.clone();
    warpPerspective(imgr,imgrt,Hi,imgrt.size(),INTER_LINEAR);
    imgrt=imgrt*0.5+imgl*0.5;
    imwrite("warpedR.png",imgrt);
    cerr << endl << "Simple rendering done";

    // find two homographies
    cerr << endl << "Start another RANSAC ...";
    Mat Hl, Hr;
    my_homographies(matches,Hl,Hr);
    cerr << " done";

    // render the output
    render(heightl,widthl,imgl,heightr,widthr,imgr,Hl,Hr,"panorama.png");
    cerr << endl << "Complex rendering done" << endl << endl;
}
