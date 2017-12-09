#include "libimse.h"
#include "image_helper.h"
#include "math_helpers.h"
#include <iostream>

void presortThread(config* cfg) {
	image* curImg = NULL;

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
		uint32_t minX, maxX, minY, maxY, size, counter;
		minX = cfg->imageResX * ((1 - cfg->maxDiameter) / 2);
		maxX = cfg->imageResX - cfg->imageResX * ((1 - cfg->maxDiameter) / 2);
		minY = cfg->imageResY * ((1 - cfg->maxDiameter) / 2);
		maxY = cfg->imageResY - cfg->imageResY * ((1 - cfg->maxDiameter) / 2);
		size = (maxX - minX + 1) * (maxY - minY + 1);
		int16_t *planet;
		counter = 0;
		int16_t val;
		planet = (int16_t*) malloc(size);
		for (uint32_t x = minX; x < maxX; x++){
			for (uint32_t y = minY; y < maxY; y++){
				cout << "size " << size << endl;
				cout << "pixel " << y*cfg->imageResX + x << endl;
				cout << "counter " << counter << endl;
				val = curImg->diffBitmap[y*cfg->imageResX + x];
				cout << "got val: " << val << endl;
				planet[counter] = val;
				cout << "set val, counter" << counter << endl;
				counter++;
				cout << "incremented counter" << endl;
			}
		}

		noise avgNoise;
		avgNoise = calcNoise16(planet, size);
		cout << avgNoise.stdDev << endl;
		double threshold = cfg->checkSNR * avgNoise.stdDev;
		
		for(uint32_t x = 0; x < size; x++) {
			if(planet[x] > threshold)
				cout << "yay " << x << planet[x] << endl;
		}
		delete[] planet;

		/* new algorithm:
			- get the planet's area of interest from the raycenter algorithm
			- calculate variance and std deviation inside this area
			- get all pixels which are more then 2(?) sigma brighter than the average
			- if more than 2 of these pixels are adjacent, this is an impact candidate
		*/
		
		if (imgIsInteresting) {
			// send the image to the next thread
			cfg->mCheck.lock();
			cfg->qCheck.push(curImg);
			cfg->statQlen5++;
			cfg->mCheck.unlock();
			cfg->statInteresting++;
			curImg = NULL;
		} else {
			// delete the image
			delete curImg;
		}
	}

	// propagate shutdown to the next thread
	cfg->shutdownThread++;
}
