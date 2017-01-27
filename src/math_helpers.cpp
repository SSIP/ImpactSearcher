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
		fwprintf(stderr, (wstring(frame->fileName) + L": moveX=" + to_wstring(move.x) + L", moveY=" + to_wstring(move.y) + L"\n").c_str());
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

double getStdDev(double variance)
{
	return sqrt(variance);
}

noise calcNoise(uint32_t *pixels) {
	double average = getAvg(pixels);
	double variance = getVariance(average, pixels);
	double stdDev = getStdDev(variance);
	return{average,variance,stdDev};
}

noise combineNoise(noise *corner1, noise *corner2, noise *corner3) {
	return{ 0.0, 0.0, 0.0 };
}

void calcNoiseCorners(image *imgData, config* cfg){
	uint32_t maxDiameter, triHeight, triLeg, numPixels;
	noise corners[4];
	
	if (cfg->imageResX > cfg->imageResY){
		maxDiameter = (uint32_t)(cfg->imageResY + cfg->imageResY * 0.2);
	}
	else {
		maxDiameter = (uint32_t)(cfg->imageResX + cfg->imageResY * 0.2);
	}
	triHeight = sqrt(2*pow(maxDiameter,2));
	triLeg = (uint32_t)sqrt(2)*triHeight;
	numPixels = (uint32_t)(0.5 * pow(2*triLeg,2));
	uint32_t *pixels = new uint32_t[numPixels];
	uint32_t y,counter;

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

	// top left
	for (uint32_t x = 0; x < triLeg; x++)
		for (uint32_t y = 0; y <= triLeg - x; y++)
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
	noise topLeft = calcNoise(pixels);
	
	counter = 0;
	// bottom left
	for (uint32_t x = 0; x < triLeg; x++)
		for (uint32_t y = cfg->imageResY - triLeg + x; y < cfg->imageResY ; y++)
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
	noise bottomLeft = calcNoise(pixels);
	
	counter = 0;
	// top right
	for (uint32_t x = cfg->imageResX - triLeg; x < cfg->imageResX; x++)
		for (uint32_t y = 0; y < (x - cfg->imageResX + triLeg); y++)
			pixels[counter++] = imgData->rawBitmap[y * cfg->imageResX + x];
	noise topRight = calcNoise(pixels);
	
	counter = 0;
	// bottom right
	for (uint32_t x = cfg->imageResX - triLeg; x < cfg->imageResX; x++)
		for (uint32_t y = cfg->imageResY - (x - cfg->imageResX + triLeg); y < cfg->imageResY; y++)
			pixels[counter] = imgData->rawBitmap[y * cfg->imageResX + x];
	noise bottomRight = calcNoise(pixels);
	
	// combine 3 variances, means of 4 times
	// each time exlcude 1 corner
	// combExcl[0 to 3]
	noise combCrnExl[4];
	combCrnExl[0] = combineNoise(topLeft, bottomLeft, bottomRight);
	combCrnExl[1] = combineNoise(bottomLeft, bottomRight, topRight);
	combCrnExl[2] = combineNoise(bottomRight, topRight, topLeft);
	combCrnExl[3] = combineNoise(topRight, topLeft, bottomLeft);
	
	// simple k.o.-system, smallest Standard Deviation wins
	
	int round1, round2, round3;
	// round #1: winner of 0 vs 1
	if(combCrnExl[0]->stdDev > combCrnExcl[1]->stdDev){
		round1 = 0;
	} else {
		round1 = 1;
	}
	
	// round #2: winner of 2 vs 3
	if(combCrnExl[2]->stdDev > combCrnExcl[3]->stdDev){
		round1 = 2;
	} else {
		round1 = 3;
	}
	
	// round #3: winner of round #1 vs winner of round #2
	if(combCrnExl[round1]->stdDev > combCrnExcl[round2]->stdDev){
		round3 = round1;
	} else {
		round3 = round2;
	}
	
	imgData->imgNoise = combCrnExl[round3];
	
	delete[] pixels;
}	
