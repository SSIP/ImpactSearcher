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

	for (y = 0; y < cfg->imageResY; y += cfg->centerSkipPixels){ // loop y axis
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
	numPixels = (uint32_t)0.5 * pow(2*triLeg,2);
	
	// take pixels in triangle beginning in corner and move in one pixel at a time
	for (n = 0; y <= triLeg; n++){
		
	}
}	

noise calcNoise(uint32_t *pixels){
	
}
