# Lab 3: Digital Image Processing

## Excercise 2-b: Histogram Equalization

Open the image `rocks.jpg` and plot the **histogram** and **CDF** of each color channel.

Apply histogram equalization using OpenCV on each color channel separately
and then combine them and show the result image.

A smarter way to do color image histogram equalization: take the average of the
3 histograms of color channel. Then use histogram matching to match each color
channel to the average histogram you computed. Combine the channels and show the
result. Compare it to the previous result.


## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./histo_color rocks.jpg
```