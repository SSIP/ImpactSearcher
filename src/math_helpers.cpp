#include "math_helpers.h"
#include <algorithm>

coordinates rayCenter(coordinates approximateCenter, image* frame, int32_t numberRays, config* cfg){
	int32_t i;
	float rayAngle = M_PI / numberRays;

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
	return coordinates{ 0, 0 };
}

coordinates massCenter(image* frame, config* cfg){
	uint32_t sumX = 0, sumY = 0, sumTotal = 0, x, y, pixel;
	coordinates move;

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
	move.x = -(int32_t)((sumX / sumTotal) - cfg->imageResX / 2);
	move.y = (sumY / sumTotal) - cfg->imageResY / 2;
	if (cfg->verbosity >= 3)
		fprintf(stderr, (string(frame->fileName) + ": moveX=" + to_string(move.x) + ", moveY=" + to_string(move.y) + "\n").c_str());
	return move;
}

double getAvg(uint32_t *pixels)
{
	uint64_t sum = 0;
	for(uint32_t x = 0; x < sizeof(pixels); x++){
		sum += pixels[x];
	}
	return sum/sizeof(pixels);
}

double getVariance(double mean, uint32_t *pixels)
{
	// is a double_t sufficient for the numbers?
	double_t temp = 0;
	for(uint32_t x = 0; x < sizeof(pixels); x++){
		temp += (pixels[x]-mean)*(pixels[x]-mean);
	}
	return temp/sizeof(pixels);
}

noise calcNoise(uint32_t *pixels) {
	noise result;
	result.average = getAvg(pixels);
	result.variance = getVariance(result.average, pixels);
	result.stdDev = sqrt(result.variance);
	result.sampleSize = sizeof(pixels);
	return result;
}

void calcNoiseCorners(image *imgData, config* cfg){
	uint32_t maxDiameter, triHeight, triLeg, numPixels;
	
	if (cfg->imageResX > cfg->imageResY){
		maxDiameter = (uint32_t)(cfg->imageResY + cfg->imageResY * 0.2);
	}
	else {
		maxDiameter = (uint32_t)(cfg->imageResX + cfg->imageResY * 0.2);
	}
	triHeight = sqrt(2*pow(maxDiameter,2));
	triLeg = (uint32_t)sqrt(2)*triHeight;
	numPixels = (uint32_t)(0.5 * pow(2*triLeg,2));
	uint32_t *pixels = new uint32_t[4][numPixels];
	uint32_t y, counter = 0;

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
	// top left
	for (uint32_t x = 0; x < triLeg; x++){
		for (uint32_t y = 0; y <= triLeg - x; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
		}
	}
	corners[0] = getAvg(pixels);
	
	counter = 0;
	// bottom left
	for (uint32_t x = 0; x < triLeg; x++) {
		for (uint32_t y = cfg->imageResY - triLeg + x; y < cfg->imageResY ; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
		}
	}
	corners[1] = getAvg(pixels);
	
	counter = 0;
	// top right
	for (uint32_t x = cfg->imageResX - triLeg; x < cfg->imageResX; x++) {
		for (uint32_t y = 0; y < (x - cfg->imageResX + triLeg); y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
		}
	}
	corners[2] = getAvg(pixels);
	
	counter = 0;
	// bottom right
	for (uint32_t x = cfg->imageResX - triLeg; x < cfg->imageResX; x++) {
		for (uint32_t y = cfg->imageResY - (x - cfg->imageResX + triLeg); y < cfg->imageResY; y++) {
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
		}
	}
	corners[4] = getAvg(pixels);
	
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
	uint32_t *totalPixels = new uint32_t[3*numPixels];
	double sum = 0;
	uint32_t m = 0;
	for(uint32_t n = 0; n < 4; n++){
		if(n == round3) {
			
		} else {
			sum = sum + corners[n];
			for(uint32_t x = 0; x < numPixels; x++){
				totalPixels[m * numPixels + x] = pixels[n * numPixels + x];
			}
			m++;
		}
	}
	double totalAvg = sum / 3;
	double variance = getVariance(totalAvg, totalPixels);
	double stdDev = sqrt(variance);
	
	delete[] totalPixels;
	delete[] pixels;
}	
