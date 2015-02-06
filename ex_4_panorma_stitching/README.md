# Panorama Stitching Code for the third Exercise "Computer Vision" WS2014/2015

The following modifications was applied to the project:

 * Many functions was replaced by OpenCV built-in functions.
 * We use a stable marriage matching approach. The matching takes a the k-nearest-
   neighbors found by a SURF filter for each keypoint and find a stable matching
   between all the keypoints.
 * We apply a quality threshold for every match. That means, "bad" matches are
   removed.


## Build

CMake is used as build tool-chain

```bash
# The OpenCV_DIR variable is only required if your OpenCV library is
# not globally available
$ cmake . -DOpenCV_DIR=/path/to/opencv/release
$ make
```

## Running times

The test environment was a Ubuntu 14.04 running as VirtualBox guest.

Characteristic:

  * AMD Phenom II X6 1100T CPU 3.30GHz (6 logical cores)
  * 3 cores was assigned to the guest system

**Original**:
compiled without `SAVE_ALL`

    $ perf stat -- ./panst images/img_0625.jpg images/img_0626.jpg 

    Images loaded. wsize_sum=8, wsize_loc=22, wsize_match=40
    Start Harris detector ...
    272 keypoints in the left image
    295 keypoints in the right image
    Start matching ...
    127 matching pairs found
    Start standard RANSAC ... done
    Simple rendering done
    Start another RANSAC ... done
    Complex rendering done


     Performance counter stats for './panst images/img_0625.jpg images/img_0626.jpg':

          60158.803310 task-clock (msec)         #    0.995 CPUs utilized          
                 1,684 context-switches          #    0.028 K/sec                  
                   153 cpu-migrations            #    0.003 K/sec                  
                17,412 page-faults               #    0.289 K/sec                        

          60.438178012 seconds time elapsed

**Modifications**:
compiled without `VERBOSE`

    $ perf stat -- ./panorama images/img_0625.jpg images/img_0626.jpg 
    Detect keypoints ...
    Non maximum suppression ...
      452 keypoints left
      399 keypoints right
    Compute feature descriptors ...
    Matching ...
    342 matches
    Remove bad matches ...
    Done
    Start RANSAC ... done
    Render ... 
    done

     Performance counter stats for './panorama images/img_0625.jpg images/img_0626.jpg':

          41578.686522 task-clock (msec)         #    0.999 CPUs utilized          
                   340 context-switches          #    0.008 K/sec                  
                    56 cpu-migrations            #    0.001 K/sec                  
                29,600 page-faults               #    0.712 K/sec                  

          41.600253332 seconds time elapsed


## Result

The running time tests show that the modifications improve the runtime of the
program. We think, that the runtime improvement comes from the usage of the
OpenCV built-in functions, rather from the stable marriage matching.

The final output was not was not improved. That means, 
We think, that the error got from matching of multiple keypoints in on image to the same
keypoint in the other image, is not that relevant. The threshold filter would remove
the "wrong" matches anyway.


## References

 * [Rosten06](http://computer-vision-talks.com/articles/2011-01-04-comparison-of-the-opencv-feature-detection-algorithms/)
