#include "libimse.h"

typedef uint8_t** imgSlice;

// defines a circle with coordinates of center plus radius
struct circle {
	coordinates center;
	double_t radius;
};

struct coordPlanet {
	double rotPoleImage; // rotation of north pole in image plane
	double poleInclination; // inclination of north pole towards observer
	bool westIsLeft; // if west is on the right side of image (after rotation) -> mirrored
};

deltacoords rayCenter(coordinates approximateCenter, image* frame, int32_t numberRays, config* cfg);
deltacoords massCenter(image* frame, config* cfg);

// fit a circle to a list of coordinates with least squares
circle fitCircle(coordinates *borderPoints);

// returns pointers to pixels in image, thereby creating a slice through
// the image. length of slice is returned as well.
bool sliceImage(coordinates start, direction vector, image imageData, uint8_t **data, uint32_t length);

// fit a simple jupiter model to image and save rotation angle and radius
// angle = 0 means north pole is facing up
bool fitJupiterModel(image *imageData);

// walk a slice from the center in both directions until threshold is reached
// threshold equals 2sigma of noise or something
coordinates *findBorder(uint32_t center, imgSlice slice);

// calculate avg and std dev of noise from corners of image
// the three corners with least difference by least squares are used for
// total background
void calcNoiseCorners(image *imgData, config *cfg);

// calculate properties of background triangles in corners
void calcCornerSize(config *cfg);

// calc noise from array of pixel values
noise calcNoise(uint32_t *pixels);

// parameter is subtracted image
// calculate noise in corners of image
// we know the area of the planet
// find pixels with unlikely deviations from mean. take adjacent pixels
// into account. add brightness of pixels if signal > 3sigma and return as signal
double signalToNoise(image *subtractedData);
