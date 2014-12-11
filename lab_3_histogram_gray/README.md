# Lab 3: Digital Image Processing

## Excercise 2a: Histogram equalization and matching

 * Open the image `spine.jpg` and plot its histogram and CDF
 * Apply histogram equalization using OpenCV and show the result.
 * Implement Histogram matching and then use it to match the histogram of this
   image to another image of your choice with a nice histogram


## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./histo_match spine.jpg
```