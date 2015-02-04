#include <iostream>
using namespace std;
#include "ps.h"

double quality(int heightl, int widthl, unsigned char *imgl, int il, int jl,
               int heightr, int widthr, unsigned char *imgr, int ir, int jr,
               int wsize) {

    // filter out points that are close to borders
    if( il-wsize<0 ) return numeric_limits<double>::max();
    if( ir-wsize<0 ) return numeric_limits<double>::max();
    if( il+wsize>heightl-1 ) return numeric_limits<double>::max();
    if( ir+wsize>heightr-1 ) return numeric_limits<double>::max();
    if( jl-wsize<0 ) return numeric_limits<double>::max();
    if( jr-wsize<0 ) return numeric_limits<double>::max();
    if( jl+wsize>widthl-1 ) return numeric_limits<double>::max();
    if( jr+wsize>widthr-1 ) return numeric_limits<double>::max();

    double q=0.;
    for( int di=-wsize; di<=wsize; di++ ) {
        for( int dj=-wsize; dj<=wsize; dj++ ) {
            int indexl=((il+di)*widthl+jl+dj)*3;
            int indexr=((ir+di)*widthr+jr+dj)*3;
            double dr=(double)imgl[indexl]-(double)imgr[indexr];
            double dg=(double)imgl[indexl+1]-(double)imgr[indexr+1];
            double db=(double)imgl[indexl+2]-(double)imgr[indexr+2];
            q+=dr*dr+dg*dg+db*db;
        }
    }
    return q;
}

vector<MATCH> matching(int heightl, int widthl, unsigned char *imgl,
                       int heightr, int widthr, unsigned char *imgr,
                       vector<KEYPOINT> pointsl, vector<KEYPOINT> pointsr, int wsize) {
    int nl=pointsl.size();
    int nr=pointsr.size();
    double *ql=new double[nl];
    double *qr=new double[nr];
    int *pl=new int[nl];
    int *pr=new int[nr];

    for( int i=0; i<nl; i++ ) {
        ql[i]=numeric_limits<double>::max();
        pl[i]=-1;
    }
    for( int i=0; i<nr; i++ ) {
        qr[i]=numeric_limits<double>::max();
        pr[i]=-1;
    }

    for( int il=0; il<nl; il++ ) {
        for( int ir=0; ir<nr; ir++ ) {
            int yl=(int)(pointsl[il].y+0.5);
            int xl=(int)(pointsl[il].x+0.5);
            int yr=(int)(pointsr[ir].y+0.5);
            int xr=(int)(pointsr[ir].x+0.5);
            double q=quality(heightl,widthl,imgl,yl,xl,heightr,widthr,imgr,yr,xr,wsize);
            if( q<ql[il] ) {
                ql[il]=q;
                pl[il]=ir;
            }
            if( q<qr[ir] ) {
                qr[ir]=q;
                pr[ir]=il;
            }
        }
    }

    // cross-check
    for( int il=0; il<nl; il++ ) {
        if( pl[il]!=-1 ) {
            if( pr[pl[il]]!=il ) pl[il]=-1;
        }
    }
    for( int ir=0; ir<nr; ir++ ) {
        if( pr[ir]!=-1 ) {
            if( pl[pr[ir]]!=ir ) pr[ir]=-1;
        }
    }

    // fill
    vector<MATCH> ret;
    ret.clear();
    for( int il=0; il<nl; il++ ) {
        if( pl[il]!=-1 ) {
            MATCH pair;
            pair.xl=pointsl[il].x;
            pair.yl=pointsl[il].y;
            pair.xr=pointsr[pl[il]].x;
            pair.yr=pointsr[pl[il]].y;
            pair.value=ql[il];
            ret.push_back(pair);
        }
    }

    return ret;
}
