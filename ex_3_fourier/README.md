# Exercise 3: Fourier approximation

In your first Exercise you extracted a specific region. Do the same with
[vertebra](Wirbel.png) to extract the cemented region in the vertebra and
approximate its contour with a variable (by slider) number of Fourier
coefficients.

For this you need contour points on equidistant values of an arc length
parameter (e.g. `s_i = i * L / 1024`).

Adapt the OpenCV findContours (or the
method of [Kovalevsky](http://www.kovalevsky.de/Topology/StraightLines_e.htm) )
to achieve this. (3P)

Creating Fourier descriptors (i.e. making the description size and rotation
independend). (+1P)

Draw your results in your input image to see how it looks / fits.


![Verbera](Wirbel.png)


## Build

```bash

# Generate Makefiles
$ cmake .

# Compile source code
$ make

# Execute program
$ ./shape Wirbel.png
```