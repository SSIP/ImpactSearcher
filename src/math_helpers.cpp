#include "math_helpers.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#define PI 3.14159265

/* Calculate the center of the planet. This requires coordinates that are already
 * within the area of the planet.
 * From this approximate center go into several directions until the signal drops
 * below 5 to 10 sigma of the background noise. This is regarded as the edge of
 * the planet.
 *
 * @approximateCenter: an estimate of the center of the planet usually calculated with massCenter function.
 * @frame:        a frame that contains the image of a planet.
 * @numberRays:   the number of rays which start from the center. 1 means 4 rays with an angle of 90 degrees
 *                between each other will be used, 2 are 8 rays with 45 degrees between each other.
 * @cfg:          a reference to the global configuration. Needed for image resolution and background noise.
 *
 * Return a struct that contains signed integers in which direction to move the image in order
 * to center the planet.
 */
deltacoords rayCenter(coordinates approximateCenter, image* curImg, uint16_t numberRays, config* cfg){
	double rayRad = PI / (numberRays * 2);
	double threshold = curImg->imgNoise.stdDev * 20 + curImg->imgNoise.average;
	cout << "new pic" << endl;
	for (uint16_t ray = 0; ray < numberRays * 4; ray ++) {
		uint32_t radius = 0;
		cout << "rad " << rayRad * ray << endl;
		while(radius <= cfg->imageResX) {
			coordinates pixel;
			radius++;
			pixel = radiusPixel(approximateCenter, rayRad * ray, radius);
			uint8_t val = curImg->rawBitmap[pixel.y * cfg->imageResX + pixel.x];
			if ( val > threshold ) {
				if (cfg->verbosity > 3) {
					curImg->rawBitmap[pixel.y * cfg->imageResX + pixel.x] = 0;
				}
			} else {
				if (cfg->verbosity > 3) {
					curImg->rawBitmap[pixel.y * cfg->imageResX + pixel.x] = 255;
				}

				break;
			}
			
		}
	}
	return deltacoords{ 0, 0 };
}

/* Calculate the center of the planet with a simple center of mass like algorithm.
 *
 * @frame:        a frame that contains the image of a planet.
 * @cfg:          a reference to the global configuration. Needed for image resolution and background noise.
 *
 * Return a struct that contains signed integers in which direction to move the image in order
 * to center the planet.
 */
deltacoords massCenter(image* frame, config* cfg){
	uint32_t sumX = 0, sumY = 0, sumTotal = 0, x, pixel;
	deltacoords move;

	for (uint32_t y = 0; y < cfg->imageResY; y += cfg->centerSkipPixels){ // loop y axis
		for (x = 0; x < cfg->imageResX; x += cfg->centerSkipPixels){ // loop x axis
			pixel = frame->rawBitmap[x + y*  cfg->imageResX];
			if (pixel >= cfg->centerThreshold){
				sumX += x*  pixel;
				sumY += y*  pixel;
				sumTotal += pixel;
			}
		}
	}

	// invert x value because of definition for moveImage function
	move.x = -(int32_t)((sumX / sumTotal) - cfg->imageResX / 2);
	move.y = (sumY / sumTotal) - cfg->imageResY / 2;
	return move;
}

/* Calculate the average value of an 8 bit unsigned integers array, usually pixel values.
 *
 * @pixels:       a array of pixel brightness values.
 * @size:         amount of pixels in the array *pixels
 *
 * Return the average of the values with double precission.
 */
double getAvg(uint8_t *pixels, uint32_t size)
{
	uint64_t sum = 0;
	for(uint32_t x = 0; x < size; x++){
		sum += pixels[x];
	}
	return (double)sum/size;
}

/* Calculate the average value of an 16 bit signed integers array, usually pixel values.
 * This function is required for the subtraction between the leading and trailing average
 * images.
 *
 * @pixels:       a array of pixel brightness values.
 * @size:         the amount of pixels in the array *pixels.
 *
 * Return the average of the values with double precission.
 */
double getAvg16(int16_t *pixels, uint32_t size)
{
	int64_t sum = 0;
	for(uint32_t x = 0; x < size; x++){
		sum += pixels[x];
	}
	return (double)sum/size;
}

/* Calculate the statistical variance of an 8 bit unsigned integers array, usually pixel values.
 *
 * @avg:          the already known average value of the values in *pixels.
 * @pixels:       a array of pixel brightness values.
 * @size:         the amount of pixels in the array *pixels.
 *
 * Return the average of the values with double precission.
 */
double getVariance(double avg, uint8_t *pixels, uint32_t size)
{
	// is a double_t sufficient for the numbers?
	double_t temp = 0;
	for(uint32_t x = 0; x < size; x++){
		temp += (pixels[x]-avg)*(pixels[x]-avg);
	}
	return temp/size;
}

/* Calculate the statistical variance of an 16 bit signed integers array, usually pixel values.
 * This function is required for the subtraction between the leading and trailing average
 * images.
 *
 * @avg:          the already known average value of the values in *pixels.
 * @pixels:       a array of pixel brightness values.
 * @size:         the amount of pixels in the array *pixels.
 *
 * Return the average of the values with double precission.
 */
double getVariance16(double avg, int16_t *pixels, uint32_t size)
{
	// is a double_t sufficient for the numbers?
	double_t temp = 0;
	for(uint32_t x = 0; x < size; x++){
		temp += (pixels[x]-avg)*(pixels[x]-avg);
	}
	return temp/size;
}

/* Calculate the noise of an 8 bit unsigned integers array, usually pixel values.
 *
 * @pixels:       a array of pixel brightness values.
 * @size:         the amount of pixels in the array *pixels.
 *
 * Return the noise struct
 */
noise calcNoise(uint8_t *pixels, uint32_t size) {
	noise result;
	result.average = getAvg(pixels, size);
	result.variance = getVariance(result.average, pixels, size);
	result.stdDev = sqrt(result.variance);
	result.sampleSize = size;
	return result;
}

/* Calculate the noise of an 16 bit signed integers array, usually pixel values.
 * This function is required for the subtraction between the leading and trailing average
 * images.
 *
 * @pixels:       a array of pixel brightness values.
 * @size:         the amount of pixels in the array *pixels.
 *
 * Return the noise struct
 */
noise calcNoise16(int16_t *pixels, uint32_t size) {
	noise result;
	result.average = getAvg16(pixels, size);
	result.variance = getVariance16(result.average, pixels, size);
	result.stdDev = sqrt(result.variance);
	result.sampleSize = size;
	return result;
}

/* Calculate the size (number of pixels) of the corners. This depends on the estimated size
 * of the planet, defined by cfg->maxDiameter.
 *
 * @cfg:          global configuration
 *
 * Return void, stores the result in the global configuration 
 */
void calcCornerSize(config *cfg){
	/* Use the shorter edge of the image and calculate the diagonale.
	 * Subtract the planet diameter and use half of the remaining
	 * corners for calculating the noise.
	 */
	uint32_t triHeight, maxDiameterPx, imageDiag;
	if (cfg->imageResY < cfg->imageResX) {
		maxDiameterPx = (uint32_t)(cfg->imageResY * cfg->maxDiameter);
		imageDiag = (uint32_t)sqrt(pow(cfg->imageResY,2) + pow(cfg->imageResY,2));
	} else {
		maxDiameterPx = (uint32_t)(cfg->imageResX * cfg->maxDiameter);
		imageDiag = (uint32_t)sqrt(pow(cfg->imageResX,2) + pow(cfg->imageResX,2));
	}

	triHeight = (imageDiag - maxDiameterPx) / 2;
	if (cfg->verbosity >= 3)
	{
		stringstream ss;
		ss << "Triangle height: " << triHeight;
		cfg->mMessages.lock();
		cfg->qMessages.push(ss.str());
		cfg->mMessages.unlock();
	}
	cfg->cornerTriLeg = (uint32_t)sqrt(2)*triHeight;
	cfg->numCornerPixels = 0;
	for(uint32_t n = 0; n <= cfg->cornerTriLeg; n++)
	{
		cfg->numCornerPixels = cfg->numCornerPixels + n;
	}
}

/* Calculate the background noise by using 3 of 4 corners. The corner with
 * the highest average value will not be used. This removes the possibilty that
 * the planet brightness will accidentially influence the background noise.
 *
 * @imgData:      array with the pixels
 * @cfg:          global configuration
 *
 * Return void, stores the result in the global configuration 
 */
void calcNoiseCorners(image *imgData, config* cfg){

	uint8_t *pixels;
	pixels = (uint8_t*) malloc (4*cfg->numCornerPixels);
	uint32_t counter = 0;

/*
 * image[x][y]
 *  0 -------------------------------------------------------> X axis
 *  |  Quadrant 2                                  Quadrant 1
 *  |  top left                                     top right
 *  |    0:0   0:1   0:2   0:3 …   0:97   0:98   0:99   0:100
 *  |    1:0   1:1   1:2       …          1:98   1:99   1:100
 *  |    2:0   2:1             …                 2:99   2:100
 *  |    3:0                   …                        3:100
 *  |     …     …     …     …  …    …      …      …      …
 *  |   97:0                   …                        7:100
 *  |   98:0  98:1             …                98:99  98:100
 *  |   99:0  99:1  99:2       …	     99:98  99:99  99:100
 *  |  100:0 100:1 100:2 100:3 … 100:97 100:98 100:99 100:100
 *  |  bottom left                               bottom right
 *  |  Quadrant 3                                  Quadrant 4
 *  v
 * 
 *  Y axis
 */

	double corners[4];
	uint32_t minX = 0, maxX = 0, minY = 0, maxY = 0, counter_start = 0;
	// top left
	minX = 0;
	maxX = cfg->cornerTriLeg;
	minY = 0;
	maxY = cfg->cornerTriLeg;
	for (uint32_t x = minX; x < maxX; x++){
		for (uint32_t y = minY; y < maxY; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
			if (cfg->verbosity >= 3)
			{
				imgData->rawBitmap[y * cfg->imageResX + x] = 100;
			}
		}
		maxY--;
	}
	corners[0] = getAvg(pixels + 0 * cfg->numCornerPixels, cfg->numCornerPixels);
	
	counter_start = counter;
	counter = cfg->numCornerPixels;
	// bottom left
	minX = 0;
	maxX = cfg->cornerTriLeg;
	minY = cfg->imageResY - cfg->cornerTriLeg - 1;
	maxY = cfg->imageResY;
	for (uint32_t x = minX; x < maxX; x++){
		minY++;
		for (uint32_t y = minY; y < maxY; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
			if (cfg->verbosity >= 4)
			{
				imgData->rawBitmap[y * cfg->imageResX + x] = 150;
			}
		}
	}
	corners[1] = getAvg(pixels + (1 * cfg->numCornerPixels), cfg->numCornerPixels);
	
	counter_start = counter;
	counter = 2*cfg->numCornerPixels;
	// top right
	minX = cfg->imageResX - cfg->cornerTriLeg;
	maxX = cfg->imageResX;
	minY = 0;
	maxY = 0;
	for (uint32_t x = minX; x < maxX; x++){
		maxY++;
		for (uint32_t y = minY; y < maxY; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
			if (cfg->verbosity >= 4)
			{
				imgData->rawBitmap[y * cfg->imageResX + x] = 200;
			}
		}
	}
	corners[2] = getAvg(pixels + (2 * cfg->numCornerPixels), cfg->numCornerPixels);
	
	counter_start = counter;
	counter = 3*cfg->numCornerPixels;
	// bottom right
	minX = cfg->imageResX - cfg->cornerTriLeg;
	maxX = cfg->imageResX;
	maxY = cfg->imageResY;
	minY = cfg->imageResY;
	for (uint32_t x = minX; x < maxX; x++){
		minY--; 
		for (uint32_t y = minY; y < maxY; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
			if (cfg->verbosity >= 4)
			{
				imgData->rawBitmap[y * cfg->imageResX + x] = 250;
			}
		}
	}
	corners[3] = getAvg(pixels + (3 * cfg->numCornerPixels), cfg->numCornerPixels);
	
	// simple k.o.-system, highest mean looses
	
	int round1 = 0, round2 = 0, round3 = 0;
	// round #1: looser of 0 vs 1
	if(corners[0] > corners[1]){
		round1 = 0;
	} else {
		round1 = 1;
	}
	
	// round #2: looser of 2 vs 3
	if(corners[2] > corners[3]){
		round2 = 2;
	} else {
		round2 = 3;
	}
	
	// round #3: looser of round #1 vs looser of round #2
	if(corners[round1] > corners[round2]){
		round3 = round1;
	} else {
		round3 = round2;
	}
	uint8_t *totalPixels;
	totalPixels = (uint8_t*) malloc (3*cfg->numCornerPixels);
	double sum = 0;
	uint32_t m = 0;
	for(uint32_t n = 0; n < 4; n++){
		if(n == round3) {
			
		} else {
			sum = sum + corners[n];
			for(uint32_t x = 0; x < cfg->numCornerPixels; x++){
				totalPixels[m * cfg->numCornerPixels + x] = pixels[n * cfg->numCornerPixels + x];
			}
			m++;
		}
	}
	imgData->imgNoise.average = sum / 3;
	imgData->imgNoise.variance = getVariance(imgData->imgNoise.average, totalPixels, 3*cfg->numCornerPixels);
	imgData->imgNoise.stdDev= sqrt(imgData->imgNoise.variance);

	delete[] totalPixels;
	delete[] pixels;
	return;
}

/* Get the n-th pixel on a radius line. angle must be >=0 and <=360
 *
 * @circleCenter:  center point of the circle
 * @rad:           angle in rad
 * @radius:        the n-th pixel on the line
 *
 * Return coordinate with x and y value of the radius pixel
 */
coordinates radiusPixel(coordinates circleCenter, double rad, uint32_t radius)

{
	double dx = 0, dy = 0, x = 0, y = 0;
	coordinates result;
	if ((rad >= 0 and rad <= 0.25 * PI) or (rad >= 1.75 * PI and rad <= 2 * PI)) {
		//iterate x up -> go right
		dy = sin(rad);
		result.y = circleCenter.y + round(radius * dy);
		result.x = circleCenter.x + radius;
	} else if (rad >= 0.75 * PI and rad <= 1.25 * PI) {
		//iterate x down -> go left
		dy = sin(rad);
		result.y = circleCenter.y + round(radius * dy);
		result.x = circleCenter.x - radius;
	} else if (rad > 0.25 * PI and rad < 0.75 * PI) {
		//iterate y up -> go up
		dx = cos(rad);
		result.y = circleCenter.y - radius;
		result.x = circleCenter.x + round(radius * dx);
	} else if (rad > 1.25 * PI and rad < 1.75 * PI) {
		//iterate y down -> go down
		dx = cos(rad);
		result.y = circleCenter.y + radius;
		result.x = circleCenter.x + round(radius * dx);
	} else {
		// something is horribly wrong
	}
	return result;
}