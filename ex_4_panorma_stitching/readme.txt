Panorama Stitching Code for the third Exercise "Computer Vision" WS2014/2015

________________________________________________________________________________

Content:

images, inf, land_col, land_normal, land_small_overlap -- examples: images + 
results

makefile -- contains only the Makefile for the Linux/OSX-build, go to the 
directory and type "make"

src -- sources (*.cpp, *.h)

VisSt2010 -- Microsoft Visual Studio 2010 Solution (Windows)

panst.pro -- qmake project, can be used from the console by typing "qmake" 
followed by "make", can be loaded into QtCreator (both Windows and Linux)
________________________________________________________________________________

Note!!! Actually I tested all the stuff under OSX 10.10.1 with gcc 4.2.1,
OpenCV 2.4.10 using "make" only. The information below is one year old (WS2013/2014):

================================================================================

All the stuff was tested under:

Windows 8.1, OpenCV 2.4.7, Microsoft Visual Studio 2010, MSVC 2010 compiler 
(32-bit), QtCreator (Qt 5.2.0 for Windows 32-bit, VS 2010)

Linux Mint 15 (Mate), OpenCV 2.4.2, gcc 4.7.3 compiler, using Makefile and 
QtCreator 2.7.0 (Qt 5.0.1, 64-bit)
________________________________________________________________________________

For Windows users:

-- OpenCV fails to compile with the actual MSVC 2012 Compiler

-- don't forget to set correct paths to the include- and library-directories

-- don't forget to adjust the System PATH in order to find the necessary dll-s
________________________________________________________________________________

Characteristic running times for an i7-2620M CPU 2.70GHz (dual-core, 4 logical 
processors), example: /land_normal/IMG_224[12]_s.JPG

Windows, QtCreator, Debug       -- 42 sec
Windows, QtCreator, Release     -- could not load images (?)
Windows, Visual Studio, Debug   -- 63 sec
Windows, Visual Studio, Release -- 18 sec
Linux, QtCreator, Debug         -- 37 sec
Linux, QtCreator, Release       -- 17 sec
Linux, Makefile (release)       -- 17 sec 
