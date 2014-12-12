#!/bin/bash
# 
# Test script for the GitLab CI.
# 
# It build the OpenCV library outside the git repository
# to speed up the build process (the library will be build
# only once per runner)

# Save the current working directory
PROJECT_DIR=$PWD
OpenCV_VERSION="2.4.9.1"

set -e

# We will build the OpenCV in the home direcory
# to prevent GitLab CI from deleting our build
cd ~

if [ ! -d "opencv" ]; then
    git clone https://github.com/Itseez/opencv.git
    cd opencv
else
    cd opencv
    git fetch --all
fi

# Checkout to the correct version. Other repositories could
# use the same repository for their builds 
git checkout $OpenCV_VERSION

# We build in a separate directory
[ ! -d "release-$OpenCV_VERSION" ] && mkdir release-$OpenCV_VERSION
cd release-$OpenCV_VERSION

# Build OpenCV library
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local .. > cmake.log
make > make.log

# Walk through all tasks
for dir_list in "$PROJECT_DIR/lab_*" "$PROJECT_DIR/exercise_*"; do
    for dir in $dir_list; do
        # Go into task directory ...
        cd $dir

        # ... and finally build the task
        cmake -D OpenCV_DIR=~/opencv/release-$OpenCV_VERSION .
        make
    done
done

