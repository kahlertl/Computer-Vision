# Task 2: Camera perspective

Create a 3D scene in OpenCV where a rectangular image is positioned so that its
centre is in the origin of the 3D coordinate system. A camera is looking always
in the direction of the image (the coordinate origin ) and gives an image of
size `512 x 512` pixels and fix the focal length to a reasonable value (for
example 100 pixels) to keep the image inside the view of 5the camera. The
position of the camera is described by spherical coordinates and controller
using three parameters that are input by the user using sliders on the main
interface. Those are :

 * r : the distance from the coordinates origin point
 * φ : the angle on the horizontal plane
 * θ : the angel with the vertical axis

Remember that the camera still needs to always be looking at the coordinate
origin point. Apply the proper computation and display the image as seen by the
camera. After that, apply an Affine transform on the image in the centre. The
input is 3 parameters also controlled by the user using sliders (with reasonable
range):

 * α : main rotation angle.
 * β : secondary rotation angle (related to the scaling).
 * λ_1 : scale factor (the other factor λ_2 = 1)

## Building

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# Create Makefile
$ cmake .

# Compile binary
$ make

# Execute
$ ./camera_perspective fruits.jpg
```
