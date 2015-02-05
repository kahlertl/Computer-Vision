# Panorama Stitching Code for the third Exercise "Computer Vision" WS2014/2015

## Content

 - example images + results:
    * [images](images/)
    * [inf](inf/)
    * [land_col](land_col/)
    * [land_normal](land_normal/)
    * [land_small_overlap](land_small_overlap/)
 - sources (*.cpp, *.h)
    * [src/](src/)

**Note!** Actually I tested all the stuff under OSX 10.10.1 with gcc 4.2.1,
OpenCV 2.4.10 using "make" only. The information below is one year old (WS2013/2014):

All the stuff was tested under:

 * Windows 8.1, OpenCV 2.4.7, Microsoft Visual Studio 2010, MSVC 2010 compiler 
   (32-bit), QtCreator (Qt 5.2.0 for Windows 32-bit, VS 2010)
 * Linux Mint 15 (Mate), OpenCV 2.4.2, gcc 4.7.3 compiler, using Makefile and 
   QtCreator 2.7.0 (Qt 5.0.1, 64-bit)


**For Windows users:**

 - OpenCV fails to compile with the actual MSVC 2012 Compiler
 - don't forget to set correct paths to the include- and library-directories
 - don't forget to adjust the System PATH in order to find the necessary dll-s


## Running times

Characteristic running times for an i7-2620M CPU 2.70GHz (dual-core, 4 logical 
processors), example: /land_normal/IMG_224[12]_s.JPG


| System                          | running times              |
| ------------------------------- | -------------------------- |
| Windows, QtCreator, Debug       | 42 sec                     |
| Windows, QtCreator, Release     | could not load images (?)  |
| Windows, Visual Studio, Debug   | 63 sec                     |
| Windows, Visual Studio, Release | 18 sec                     |
| Linux, QtCreator, Debug         | 37 sec                     |
| Linux, QtCreator, Release       | 17 sec                     |
| Linux, Makefile (release)       | 17 sec                     |
# Panorama Stitching Code for the third Exercise "Computer Vision" WS2014/2015

## Content

 - example images + results:
    * [images](images/)
    * [inf](inf/)
    * [land_col](land_col/)
    * [land_normal](land_normal/)
    * [land_small_overlap](land_small_overlap/)
 - sources (*.cpp, *.h)
    * [src/](src/)

**Note!** Actually I tested all the stuff under OSX 10.10.1 with gcc 4.2.1,
OpenCV 2.4.10 using "make" only. The information below is one year old (WS2013/2014):

## Build

If you prefere to use `cmake` you can run:

```bash
$ cmake . -DOpenCV_DIR=/path/to/opencv/release
$ make -f Makefile
```

Otherwise - if OpenCV is globally installed on your system - you can run the hand
written Makefile

```bash
$ cd makefile/
$ make
```

All the stuff was tested under:

 * Windows 8.1, OpenCV 2.4.7, Microsoft Visual Studio 2010, MSVC 2010 compiler 
   (32-bit), QtCreator (Qt 5.2.0 for Windows 32-bit, VS 2010)
 * Linux Mint 15 (Mate), OpenCV 2.4.2, gcc 4.7.3 compiler, using Makefile and 
   QtCreator 2.7.0 (Qt 5.0.1, 64-bit)


**For Windows users:**

 - OpenCV fails to compile with the actual MSVC 2012 Compiler
 - don't forget to set correct paths to the include- and library-directories
 - don't forget to adjust the System PATH in order to find the necessary dll-s


## Running times

Characteristic running times for an i7-2620M CPU 2.70GHz (dual-core, 4 logical 
processors), example: /land_normal/IMG_224[12]_s.JPG


| System                          | running times              |
| ------------------------------- | -------------------------- |
| Windows, QtCreator, Debug       | 42 sec                     |
| Windows, QtCreator, Release     | could not load images (?)  |
| Windows, Visual Studio, Debug   | 63 sec                     |
| Windows, Visual Studio, Release | 18 sec                     |
| Linux, QtCreator, Debug         | 37 sec                     |
| Linux, QtCreator, Release       | 17 sec                     |
| Linux, Makefile (release)       | 17 sec                     |


## References

 * [Rosten06](http://computer-vision-talks.com/articles/2011-01-04-comparison-of-the-opencv-feature-detection-algorithms/)
