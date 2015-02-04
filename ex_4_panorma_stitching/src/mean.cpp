#include <stdio.h>
#include <iostream>
using namespace std;
#include "ps.h"

void mean_filter(int height, int width, double *a, int wsize) {
    double *tmp=new double[height*width];

    // horizontal
    for( int i=0; i<height; i++ ) {
        // integrate
        double *row=new double[width];
        row[0]=a[i*width];
        for( int j=1; j<width; j++ ) row[j]=row[j-1]+a[i*width+j];
        // left border
        for( int j=0; j<wsize+1; j++ ) tmp[i*width+j]=row[j+wsize]/(j+wsize+1.);
        // inside
        for( int j=wsize+1; j<width-wsize; j++ ) tmp[i*width+j]=(row[j+wsize]-row[j-wsize-1])/(2.*wsize+1.);
        // right border
        for( int j=width-wsize; j<width; j++ ) tmp[i*width+j]=(row[width-1]-row[j-wsize-1])/(width-j+wsize);
        delete row;
    }

    // vertical
    for( int j=0; j<width; j++ ) {
        // integrate
        double *col=new double[height];
        col[0]=tmp[j];
        for( int i=1; i<height; i++ ) col[i]=col[i-1]+tmp[i*width+j];
        // top border
        for( int i=0; i<wsize+1; i++ ) a[i*width+j]=col[i+wsize]/(i+wsize+1.);
        // inside
        for( int i=wsize+1; i<height-wsize; i++ ) a[i*width+j]=(col[i+wsize]-col[i-wsize-1])/(2.*wsize+1.);
        // bottom border
        for( int i=height-wsize; i<height; i++ ) a[i*width+j]=(col[height-1]-col[i-wsize-1])/(height-i+wsize);
        delete col;
    }

    delete tmp;
}
