#include <stdio.h>
#include <iostream>
#include <set>
using namespace std;
#include "ps.h"

vector<KEYPOINT> local_maxima(int height, int width, double *e, int wsize, const char *name) {
    // separable maximum filter
    double *tmp1=new double[height*width];
    double *tmp2=new double[height*width];
    // horizontal
    for( int i=0; i<height; i++ ) {
        multiset<double> set;
        set.clear();
        // prefill the multiset
        for( int j=0; j<wsize; j++ ) set.insert(e[i*width+j]);
        // left border
        for( int j=0; j<wsize+1; j++ ) {
            set.insert(e[i*width+j+wsize]);
            tmp1[i*width+j]=*(set.rbegin());
        }
        // inside
        for( int j=wsize+1; j<width-wsize; j++ ) {
            set.erase(set.find(e[i*width+j-wsize-1]));
            set.insert(e[i*width+j+wsize]);
            tmp1[i*width+j]=*(set.rbegin());
        }
        // right border
        for( int j=width-wsize; j<width; j++ ) {
            set.erase(set.find(e[i*width+j-wsize-1]));
            tmp1[i*width+j]=*(set.rbegin());
        }
    }
    // vertical
    for( int j=0; j<width; j++ ) {
        multiset<double> set;
        set.clear();
        // prefill the multiset
        for( int i=0; i<wsize; i++ ) set.insert(tmp1[i*width+j]);
        // top border
        for( int i=0; i<wsize+1; i++ ) {
            set.insert(tmp1[(i+wsize)*width+j]);
            tmp2[i*width+j]=*(set.rbegin());
        }
        // inside
        for( int i=wsize+1; i<height-wsize; i++ ) {
            set.erase(set.find(tmp1[(i-wsize-1)*width+j]));
            set.insert(tmp1[(i+wsize)*width+j]);
            tmp2[i*width+j]=*(set.rbegin());
        }
        // bottom border
        for( int i=height-wsize; i<height; i++ ) {
            set.erase(set.find(tmp1[(i-wsize-1)*width+j]));
            tmp2[i*width+j]=*(set.rbegin());
        }
    }

#ifdef SAVE_ALL
    save_double_as_image(height,width,tmp2,(string("m")+string(name)+string(".png")).c_str());
#endif

    // find local maxima
    vector<KEYPOINT> ret;
    ret.clear();

    for( int i=wsize+2; i<height-wsize-2; i++ ) {
        for( int j=wsize+2; j<width-wsize-2; j++ ) {
            if( e[i*width+j]==tmp2[i*width+j] ) {
                KEYPOINT point;
                point.x=j;
                point.y=i;
                point.value=tmp2[i*width+j];
                ret.push_back(point);
            }
        }
    }

    delete tmp1;
    delete tmp2;

    return ret;
}
