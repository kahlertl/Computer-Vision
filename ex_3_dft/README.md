# Exercise 3: Fourier transformation

Find the position and orientation of the text lines in the image
[txt.png](txt.png). Solve this problem with Fourier methods. Output: mark each
of the lines with a red bounding box which has a height of half of the distance
of the text lines. (2P)

Try to avoid digitalisation problems in the Fourier image to get a more accurate
line segmentation! (+1P)

![Text lines](txt.png)


## Build

```bash

# Generate Makefiles
$ cmake .

# Compile source code
$ make

# Execute program
$ ./magnitude txt.png
```