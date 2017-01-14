#include "math_helpers.h"
#include "definitions.h"

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

void calcNoiseCorners(image *imgData, config* cfg){
	uint32_t maxDiameter, triHeight, triLeg, numPixels;
	noise upperLeft, upperRight, bottomLeft, bottomRight;
	
	if (cfg->imageResX > cfg->imageResY){
		maxDiameter = (uint32_t)(cfg->imageResY + cfg->imageResY * 0.2);
	}
	else {
		maxDiameter = (uint32_t)(cfg->imageResX + cfg->imageResY * 0.2);
	}
	triHeight = sqrt(2*pow(maxDiameter,2));
	triLeg = (uint32_t)sqrt(2)*triHeight;
	numPixels = (uint32_t)(0.5 * pow(2*triLeg,2));
	uint32_t pixels[numPixels];
	uint32_t y,counter;
/*
 *   0:0   0:1   0:2    0:3 …   0:97   0:98   0:99   0:100
 *   1:0   1:1   1:2        …          1:98   1:99   1:100
 *   2:0   2:1              …                 2:99   2:100
 *   3:0                    …                        3:100
 *    …     …     …      …  …    …      …      …      …
 *  97:0                    …                        7:100
 *  98:0  98:1              …                98:99  98:100
 *  99:0  99:1  99:2        …	      99:98  99:99  99:100
 * 100:0 100:1 100:2  100:3 … 100:97 100:98 100:99 100:100
 */
	// upper left corner
	// take pixels in triangle beginning in corner and move in one pixel at a time
	for (uint32_t n = 0; n <= triLeg; n++){
		for (uint32_t x = 0; x < triLeg; x++){
			// sum of x and y coords equals n, split between x and y
			y = n - x;
			pixels[counter] = imgData->rawBitmap[x][y];
			counter++;
		}
	}
	upperLeft = calcNoise(pixels);
	
	counter = 0;
	// bottom left
	for (uint32_t n = cfg->imageResY; n <= triLeg; n--){
		for (uint32_t y = 0; y < triLeg; y++) {
			// sum of x and y coords equals n, split between x and y
			x = n - y;
			pixels[counter] = imgData->rawBitmap[x][y];
			counter++;
		}
	}
	bottomLeft = calcNoise(pixels);
}	

noise calcNoise(uint32_t *pixels){
	
}
