#include "math_helpers.h"
#include <algorithm>
#include <sstream>

deltacoords rayCenter(coordinates approximateCenter, image* frame, int32_t numberRays, config* cfg){
	int32_t i;
	float rayAngle = (float)M_PI / numberRays;

	//turn axes of coordinates i times by rayAngle
	for (i = 1; i < numberRays; i++){
		int32_t brightestValue, radius;

		//initial value of center pixel
		brightestValue = frame->rawBitmap[approximateCenter.x + approximateCenter.y*  cfg->imageResX];

		radius = 1;

		int32_t stopX(0), stopY(0);

		//increase radius by one, check if pixel is brighter. if brightness < threshold, jupiter radius has been reached
		while (stopX == 0 || stopY == 0){
			//cross section in "x" direction

			//cross section in "y" direction

			radius++;
		}
	}
	return deltacoords{ 0, 0 };
}

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
	coordinates centerOfMass, center;
	centerOfMass.x = (sumX / sumTotal);
	centerOfMass.y = (sumY / sumTotal);
	center.x = cfg->imageResX / 2;
	center.y = cfg->imageResY / 2;
	// invert x value because of definition for moveImage function
	move.x = -(int32_t)((sumX / sumTotal) - cfg->imageResX / 2);
	move.y = (sumY / sumTotal) - cfg->imageResY / 2;
	return move;
}

// Params: Array containing pixel values, 
double getAvg(uint8_t *pixels, uint32_t length)
{
	uint64_t sum = 0;
	for(uint32_t x = 0; x < length; x++){
		sum += pixels[x];
	}
	return (double)sum/length;
}

double getVariance(double mean, uint8_t *pixels)
{
	// is a double_t sufficient for the numbers?
	double_t temp = 0;
	for(uint32_t x = 0; x < sizeof(pixels); x++){
		temp += (pixels[x]-mean)*(pixels[x]-mean);
	}
	return temp/sizeof(pixels);
}

noise calcNoise(uint8_t *pixels) {
	noise result;
	result.average = getAvg(pixels, sizeof(pixels));
	result.variance = getVariance(result.average, pixels);
	result.stdDev = sqrt(result.variance);
	result.sampleSize = sizeof(pixels);
	return result;
}

void calcCornerSize(config *cfg){
	uint32_t triHeight, maxDiameterPx;
	maxDiameterPx = (uint32_t)(cfg->imageResY * cfg->maxDiameter);
	triHeight = (uint32_t)sqrt(2*pow(maxDiameterPx,2))/2;

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
			if (cfg->verbosity >= 3)
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
			if (cfg->verbosity >= 3)
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
			if (cfg->verbosity >= 3)
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
	imgData->imgNoise.variance = getVariance(imgData->imgNoise.average, totalPixels);
	imgData->imgNoise.stdDev= sqrt(imgData->imgNoise.variance);

	delete[] totalPixels;
	delete[] pixels;
	return;
}
