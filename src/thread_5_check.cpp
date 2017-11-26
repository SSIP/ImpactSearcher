#include "libimse.h"

void OneDToTwoD(int32_t i, coordinates* retXY, config* cfg) {
	retXY->x = i % cfg->imageResX;
	retXY->y = (uint32_t)floor(i / cfg->imageResX);
}

int32_t TowDtoOneD(int32_t x, int32_t y, config* cfg)
{
	int32_t retI;
	retI = x + y*  cfg->imageResX;
	return retI;
}

void checkThread(config* cfg) {
	image* curImg = NULL;

	for (; cfg->shutdownThread != 5; this_thread::sleep_for(chrono::milliseconds(10))) {
		// wait for the ui
		cfg->mUiPresort.lock();
		cfg->mUiPresort.unlock();
		// get the next image from our queue
		cfg->mCheck.lock();
		if(cfg->qCheck.empty()) {
			cfg->mCheck.unlock();
			continue;
		}
		else {
			curImg = cfg->qCheck.front();
			cfg->qCheck.pop();
			cfg->statQlen5--;
			cfg->mCheck.unlock();
		}

		// curImg now contains the current image ready for checking and reporting

		int8_t foundNewROI = 1;
		int8_t* flagedPixels = new int8_t[cfg->imageResX*  cfg->imageResY];
		
		while (foundNewROI == 1) {
			impact curImpact;
			foundNewROI = 0;
			//first lets find the brightest pixel which is not already in a region of interest, brighter than all other pixels and at least as bright as value found in the presort
			for (uint32_t i = 0; i < (cfg->imageResX*  cfg->imageResY); i++) {
				if (curImpact.maxVal < curImg->diffBitmap[i] && flagedPixels[i] == 0 && curImg->diffBitmap[i] >= curImg->interestingStartValue) {
					curImpact.center.x = i % cfg->imageResX;
					curImpact.center.y = (uint32_t)floor(i / cfg->imageResX);
					curImpact.maxVal = curImg->diffBitmap[i];
					foundNewROI = 1;
					flagedPixels[i] = 1;
				}
			}
			if (foundNewROI == 1) {
				//now we need to find all pixels around the center with a brightness above 3dB of maxVal and flag dem as ROI
				uint32_t radius = 1;

				int8_t foundPixel = 1;

				while (foundPixel == 1) { //increase radius
					//top left pixel
					coordinates topLeft;
					topLeft.x = curImpact.center.x - radius;
					topLeft.y = curImpact.center.y + radius;

					//bottom right pixel
					coordinates bottomRight;
					bottomRight.x = curImpact.center.x + radius;
					bottomRight.y = curImpact.center.y - radius;

					curImpact.radius = radius;

					for (uint32_t r = 1; r <= radius; r++) {

						//top boundary: increase x from topLeft
						if (curImg->diffBitmap[TowDtoOneD(topLeft.x + r, topLeft.y, cfg)] >= 0.71*  curImpact.maxVal && flagedPixels[TowDtoOneD(topLeft.x + r, topLeft.y, cfg)] == 0) {
							flagedPixels[TowDtoOneD(topLeft.x + r, topLeft.y, cfg)] = 1;
							curImpact.ROI.push_back(coordinates{ topLeft.x + r, topLeft.y });
							curImpact.totalLum += curImg->diffBitmap[TowDtoOneD(topLeft.x + r, topLeft.y, cfg)];
						}

						//bottom boundary: decrease x from bottomRight
						if (curImg->diffBitmap[TowDtoOneD(bottomRight.x - r, bottomRight.y, cfg)] >= 0.71*  curImpact.maxVal && flagedPixels[TowDtoOneD(bottomRight.x - r, bottomRight.y, cfg)] == 0) {
							flagedPixels[TowDtoOneD(bottomRight.x - r, bottomRight.y, cfg)] = 1;
							curImpact.ROI.push_back(coordinates{ bottomRight.x - r, bottomRight.y });
							curImpact.totalLum += curImg->diffBitmap[TowDtoOneD(bottomRight.x - r, bottomRight.y, cfg)];
						}

						//left boundary: decrease y from topLeft
						if (curImg->diffBitmap[TowDtoOneD(topLeft.x, topLeft.y - r, cfg)] >= 0.71*  curImpact.maxVal && flagedPixels[TowDtoOneD(topLeft.x, topLeft.y - r, cfg)] == 0) {
							flagedPixels[TowDtoOneD(topLeft.x, topLeft.y - r, cfg)] = 1;
							curImpact.ROI.push_back(coordinates{ topLeft.x, topLeft.y - r });
							curImpact.totalLum += curImg->diffBitmap[TowDtoOneD(topLeft.x, topLeft.y - r, cfg)];
						}

						//right boundary: increase y from bottomRIght
						if (curImg->diffBitmap[TowDtoOneD(bottomRight.x, bottomRight.y + r, cfg)] >= 0.71*  curImpact.maxVal && flagedPixels[TowDtoOneD(bottomRight.x, bottomRight.y + r, cfg)] == 0) {
							flagedPixels[TowDtoOneD(bottomRight.x, bottomRight.y + r, cfg)] = 1;
							curImpact.ROI.push_back(coordinates{ bottomRight.x, bottomRight.y + r });
							curImpact.totalLum += curImg->diffBitmap[TowDtoOneD(bottomRight.x, bottomRight.y + r, cfg)];
						}

					}
					radius += 1;
				}
			}
			
		}
		/* //code of old version with snr calc in thread 4
		double squareDiffSumVal = 0;
		//calculate variance
		for (uint32_t i = 0; i < (cfg->imageResX*  cfg->imageResY); i++) {
			squareDiffSumVal += pow(curImg->rawBitmap[i] - curImg->avgValue, 2);
		}
		
		//and the variance is:
		curImg->varianceValue = squareDiffSumVal / (cfg->imageResX*  cfg->imageResY);
		curImg->stdDev = sqrt(curImg->varianceValue);

		if (cfg->verbosity >= 2)
			WARNX((std::string(curImg->fileName) + ": stdDev=" + std::to_string(curImg->stdDev)).c_str());

		//check for interesting pixels
		bool imgIsInteresting = false;
		for (uint32_t i = 0; i < (cfg->imageResX*  cfg->imageResY); i++) {
			if (curImg->diffBitmap[i] >= curImg->avgValue + (cfg->presortSigma*  curImg->stdDev)) {
				curImpact.center.x = i % cfg->imageResX;
				curImpact.center.y = floor(i / cfg->imageResX);
				curImpact.maxVal = curImg->rawBitmap[i];
				curImg->impacts->push_front(curImpact);
				imgIsInteresting = true;
			}
		}
		*/
		// finished
		curImg = NULL;
	}

	// propagate shutdown to the main function
	cfg->shutdownThread++;
}
