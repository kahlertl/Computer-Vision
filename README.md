# Introduction to Computer Vision

[![build status](https://square-src.de/ci/projects/5/status.png?ref=master)](https://square-src.de/ci/projects/5?ref=master)

Repository for the source code of my internship [Intro to CV](http://www.inf.tu-dresden.de/index.php?node_id=3487&ln=en)
in 2014/15 at the TU Dresden.

## Build

To build the different tasks, you need the OpenCV library version `2.4.9`. After
that, you can use `cmake` to create a `Makefile` and build the project. Just go
into a certain task directory

```bash
# Go into task directory, for example task 1
cd task_1_colored_regions

# Generate Makefile
cmake .

# Build the project
make
```