# Lab 3: Digital Image Processing

## Excercise 3-a: Unsharp Masking

Give a `3×3` mask for performing unsharp masking in a single pass through an
image. Demostrate it works with an example .

## Build

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# generate Makefile
$ cmake .

# compile binary
$ make

# execute
$ ./unsharp man.jpg
```