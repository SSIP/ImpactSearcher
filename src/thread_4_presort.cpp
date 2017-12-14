#include "libimse.h"
#include "image_helper.h"
#include "math_helpers.h"
#include <iostream>
#include <sstream>

void presortThread(config* cfg) {
	image* curImg = NULL;
	int16_t *planet;
	bool firstLoop = true;
	// Calculate a box around the centered planet, based on the maximum expected diameter
	// This reduces the workload for calculating the noise and also excludes background.
	// (best case) TODO: get area of planet (circle) and use only that.
	uint32_t minX, maxX, minY, maxY, size, counter, border;
	if(cfg->imageResX > cfg->imageResY)
		border = cfg->imageResY;
	else
		border = cfg->imageResX;
	border = border * (1 - cfg->maxDiameter) / 2;
	minX = border;
	maxX = cfg->imageResX - border;
	minY = border;
	maxY = cfg->imageResY - border;
	size = (maxX - minX) * (maxY - minY);
	if (cfg->verbosity >= 0)
	{
		stringstream ss;
		ss << "size " << size << " minX " << minX << " maxX " << maxX << " minY " << minY << " maxY " << maxY;
		cfg->mMessages.lock();
		cfg->qMessages.push(ss.str());
		cfg->mMessages.unlock();
	}
	for (; cfg->shutdownThread != 4; this_thread::sleep_for(chrono::milliseconds(10))) {
		// wait for the ui
		cfg->mUiAverage.lock();
		cfg->mUiAverage.unlock();
		// get the next image from our queue
		cfg->mPresort.lock();

		if(cfg->qPresort.empty()) {
			cfg->mPresort.unlock();
			continue;
		}
		else {
			curImg = cfg->qPresort.front();
			cfg->qPresort.pop();
			cfg->statQlen4--;
			cfg->mPresort.unlock();
		}

		// curImg now contains the current image ready for presorting
		bool imgIsInteresting = false;

		// copy the section of the diff image that contains the planet
		counter = 0;
		int16_t val;
		planet = (int16_t*) malloc(2 * size);
		for (uint32_t x = minX; x < maxX; x++){
			for (uint32_t y = minY; y < maxY; y++){
				planet[counter] = curImg->diffBitmap[y*cfg->imageResX + x];
				counter++;
			}
		}

		curImg->diffNoise = calcNoise16(planet, size);
		double threshold = curImg->diffNoise.average + cfg->checkSNR * curImg->diffNoise.stdDev;
		
		int16_t max;
		curImg->candidate = -32768;
		for(uint32_t x = 0; x < size; x++) {
			if(planet[x] > threshold && planet[x] > curImg->candidate) {
				curImg->candidate = planet[x];
			}
		}
		if (cfg->verbosity >= 2 && curImg->candidate != -32768)
		{
			stringstream ss;
			ss << "Candidate image: " << curImg->frameNo << ", average: " << curImg->diffNoise.average << ", standard deviation: " << curImg->diffNoise.stdDev << ", value: " << curImg->candidate;
			cfg->mMessages.lock();
			cfg->qMessages.push(ss.str());
			cfg->mMessages.unlock();
		}
		free(planet);

		/* new algorithm:
			- get the planet's area of interest from the raycenter algorithm
			- calculate variance and std deviation inside this area
			- get all pixels which are more then 2(?) sigma brighter than the average
			- if more than 2 of these pixels are adjacent, this is an impact candidate
		*/
		
		//if (imgIsInteresting) {
		if(firstLoop){
			thread(checkThread, cfg).detach();
			firstLoop = false;
		}
		// send the image to the next thread
		cfg->mCheck.lock();
		cfg->qCheck.push(curImg);
		cfg->statQlen5++;
		cfg->mCheck.unlock();
		cfg->statInteresting++;
		curImg = NULL;
		/*} else {
			// delete the image
			delete curImg;
		}*/
	}

	// propagate shutdown to the next thread
	cfg->shutdownThread++;
}
