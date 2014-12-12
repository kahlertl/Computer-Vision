# Lab 3: Digital Image Processing

## Excercise 1: Gray transformation

Create an OpenCV program that takes an image (spine.jpg for example) and applies
a gray transformation

 * Log transformation (with parameter α)
 * Power-low transformation (with parameter Υ)
 * Another transformation of your choice


## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./gray_trans fruits.jpg
```