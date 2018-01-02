#include "libimse.h"

struct coordPlanet {
	double rotPoleImage; // rotation of north pole in image plane
	double poleInclination; // inclination of north pole towards observer
	bool westIsLeft; // if west is on the right side of image (after rotation) -> mirrored
};

deltacoords rayCenter(coordinates approximateCenter, image* frame, uint16_t numberRays, config* cfg);
deltacoords massCenter(image* frame, config* cfg);

double getAvg(uint8_t *pixels, uint32_t size);
double getAvg16(int16_t *pixels, uint32_t size);

double getVariance(double avg, uint8_t *pixels, uint32_t size);
double getVariance16(double avg, int16_t *pixels, uint32_t size);

noise calcNoise(uint8_t *pixels, uint32_t size);
noise calcNoise16(int16_t *pixels, uint32_t size);

void calcCornerSize(config *cfg);

void calcNoiseCorners(image *imgData, config *cfg);

coordinates radiusPixel(coordinates circleCenter, double rad, uint32_t radius);
