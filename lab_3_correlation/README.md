# Lab 3: Digital Image Processing

## Excercise 4: Find my dog

Write an openCV function that computes the correlation between a template image
and every patch of same size in the target image.

Use this to find the location of the patch ([dog.jpg]())
in the image ([phantomdogs.jpg]()).

Do not use the built in openCV functions for template matching (even though you
can look at the tutorials to see how they are used).


## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./correlation dog.jpg phantomdogs.jpg
```