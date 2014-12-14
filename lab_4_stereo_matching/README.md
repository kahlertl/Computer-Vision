# Lab 4: Stereo Matching

## Block Matching

 * Define a 2D block around each pixel
 * Search in the corresponding line for best matching block
 * Compute the disparity of each pixel

### Matching criteria

**SSD**
    * sum of square differences
    * Grayscale or Color (`L * a * b * color space`)

**ASD**
    * Absolute sum of differences
    * Grayscale or Color (`L * a * b * color space`)

**Cross correlation**
    * See [lab 3](../lab_3_cross_correlation/)


### Block Size and Max disparity

 * Try different block sizes and report accuracy on each one
   (4, 8, 16, 32, ...)
 * Try different Max dispartity and report accuracy for each one


## Post Processing

 * Median Filtering
     - Remove small outliers
 * Left-right consistency
     - Check that flow on left and right is simiar and detect occlusion
       regions
 * In Paint occlusion regions
     - Use nearest neighbor or plane fitting
 * (*Optional*) Sub-pixel accuracy


## Task 2

 * For each pixel, find the block size that gives the best results.
   Visualize the results.
 * Fix the optional block size for each pixel (variable block size) and
   repeat the matching. Report the result accuracy.


## Task 3

Let us consider that we know the following information:

 - Displacement between the two stero pairs is `8cm`
 - Depth of the orange lamp with respect to the camera is `2.8m`

Use appropriate geometry (a simple equation) to compute and the distance
for each pixel from the disparity value.

Display colored point cloud.