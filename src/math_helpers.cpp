#include "math_helpers.h"
#include "definitions.h"
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
	for (uint32_t x = 0; x < triLeg; x++){
		for (uint32_t y = 0; y <= triLeg - x; y++){
			pixels[counter] = imgData->rawBitmap[y][x];
			counter++;
		}
	}
	topLeft = calcNoise(pixels);
	
	counter = 0;
	// bottom left
	for (uint32_t x = 0; x < triLeg; x++){
		for (uint32_t y = cfg-imageResY - triLeg + x; y < cfg->imageResY ; y++){
			pixels[counter] = imgData->rawBitmap[y][x];
			counter++;
		}
	}
	bottomLeft = calcNoise(pixels);
	
	counter = 0;
	// top right
	for (uint32_t x = cfg->imageResX - triLeg; x < cfg->imageResX; x++){
		for (uint32_t y = 0; y < (x - cfg->imageResX + triLeg); y++){
			pixels[counter] = imgData->rawBitmap[y][x];
			counter++;
		}
	}
	topRight = calcNoise(pixels);
	
	counter = 0;
	// bottom right
	for (uint32_t x = cfg->imageResX - triLeg; x < cfg->imageResX; x++){
		for (uint32_t y = cfg->imageResY - (x - cfg->imageResX + trileg); y < cfg->imageResY; y++){
			pixels[counter] = imgData->rawBitmap[y][x];
			counter++;
		}
	}
	bottomRight = calcNoise(pixels);
	
	// least squares differences in avg and std dev, use the 3 closest corners
	int myints[] = {1,2,3,4};

	std::sort (myints,myints+4);

	do {
		// calculate "sum" of averages and variances
	} while ( std::next_permutation(myints,myints+4) );

	delete[] pixels;
}	

noise calcNoise(uint32_t *pixels){

}

double getMean()
{
	double sum = 0.0;
	for(double a : data)
		sum += a;
	return sum/size;
}

double getVariance()
{
	double mean = getMean();
	double temp = 0;
	for(double a :data)
		temp += (a-mean)*(a-mean);
	return temp/size;
}

double getStdDev()
{
	return Math.sqrt(getVariance());
}
