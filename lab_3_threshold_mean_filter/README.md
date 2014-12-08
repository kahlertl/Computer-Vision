# Lab 3: Digital Image Processing

## Excercise 3b: Threshold mean filter

A threshold operation is applied to the median filter such that the filter is
only activated if the data within the filter window contain a grayscale level
above some threshold value.

This program uses parts of my solution of exercise 2 of the Computer Vision 1
course (fast median filter).

## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./threshold_mean_filter fruits.jpg
```