#include "definitions.h"

// defines a circle with coordinates of center plus radius
struct circle {
	coordinates center;
	double_t radius;
};

coordinates rayCenter(coordinates approximateCenter, image* frame, int32_t numberRays, config* cfg);
coordinates massCenter(image* frame, config* cfg);

// fit a circle to a list of coordinates with least squares
circle fitCircle(coordinates *borderPoints);

// returns pointers to pixels in image, thereby creating a slice through
// the image. length of slice is returned as well.
bool sliceImage(coordinates start, direction vector, image imageData, uint8_t **data, uint32_t length);

// fit a simple jupiter model to image and save rotation angle and radius
bool fitJupiterModel(image *imageData);

// walk a slice from the center in both directions until threshold is reached
// threshold equals 2sigma of noise or something
coordinates *findBorder(uint32_t center, imgSlice slice);

// calculate avg and std dev of noise from corners of image
// the three corners with least difference by least squares are used for
// total background
void calcNoise(image *imgData);
