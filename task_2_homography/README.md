# Task 2: Homography

Sometimes in sport event broadcasting the names of teams, players or
advertisement images are projected on the field with the right perspective to
fit the images from the camera. Create an OpenCV program that takes two images
as input. One is a perspective photo of a sport field, a swimming pool or any
other arena. The other is a flat image of some logo or adver- tisement. You can
chose ny clicking any 4 points on the photo of the arena and the logo image
should be transformed and overplayed over the original photo.

## Building

The project has no other dependencies than OpenCV v2.4.9. You can build the
project with `cmake`:

```bash
# Create Makefile
$ cmake .

# Compile binary
$ make

# Execute
$ ./homography pool.jpg logo.png
```

## Click order

The follwing ordering is assumed if you click the corner points (mathematical
positive direction of rotation):

1. upper left corner
2. lower left corner
3. lower right corner
4. upper right corner