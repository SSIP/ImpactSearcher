#include "libimse.h"
#include "image_helper.h"
#include "math_helpers.h"
#include <sstream>
#include <iostream>

void moveImage(config* cfg, image* curImg, int32_t moveX, int32_t moveY) {
	// crop centered image
	cout << "move x " << moveX << " move y " << moveY << endl;
	// postive values of moveY indicate that the center of the image should be moved up, negative values indicate moving down
	if (moveY < 0) {
		memmove(curImg->rawBitmap + -moveY*  cfg->imageResX, curImg->rawBitmap, cfg->imageResX*  (cfg->imageResY + moveY));
		memset(curImg->rawBitmap, 0, cfg->imageResX*  -moveY);
	}
	else if (moveY > 0) {
		memmove(curImg->rawBitmap, curImg->rawBitmap + moveY*  cfg->imageResX, cfg->imageResX*  (cfg->imageResY - moveY));
		memset(curImg->rawBitmap + (cfg->imageResY - moveY)*  cfg->imageResX, 0, cfg->imageResX*  moveY);
	}

	// postive values of moveX indicate that the center of the image should be moved to the right, negative values indicate moving to the left
	if (moveX < 0)
		for (int32_t y = 0; y < (int32_t) cfg->imageResY; y++) {
			memmove(curImg->rawBitmap + y*  cfg->imageResX, curImg->rawBitmap + y*  cfg->imageResX - moveX, cfg->imageResX + moveX);
			for (int32_t i = 0; i > moveX; i--)
				*(curImg->rawBitmap + y*  cfg->imageResX + cfg->imageResX + i - 1) = 0;
		}
	else if (moveX > 0)
		for (int32_t y = cfg->imageResY - 1; y >= 0; y--) {
			memmove(curImg->rawBitmap + y*  cfg->imageResX + moveX, curImg->rawBitmap + y*  cfg->imageResX, cfg->imageResX - moveX);
			for (int32_t i = 0; i < moveX; i++)
				*(curImg->rawBitmap + y*  cfg->imageResX + i) = 0;
		}
}

void centerThread(config* cfg) {
	image* curImg = NULL;
	bool queueEmpty;
	bool firstAvg = false;
	int32_t firstAvgCount = 0;
	calcCornerSize(cfg);
	for (; cfg->shutdownThread != 2; this_thread::sleep_for(chrono::milliseconds(10))) {
		// get the next image from our queue
		cfg->mCenter.lock();

		if(cfg->qCenter.empty()) {
			cfg->mCenter.unlock();
			continue;
		}
		else {
			curImg = cfg->qCenter.front();
			cfg->qCenter.pop();
			cfg->statQlen2--;
			cfg->mCenter.unlock();
		}

		// curImg now contains the current image ready for centering
		// remark: hotpixel recognition was decided to be not needed because of the minimal impact it has after centering and averaging

		deltacoords moveCenter;
		coordinates approxCenter;
		// rough center estimation with center of mass logic
		moveCenter = massCenter(curImg, cfg);

		approxCenter.x = (cfg->imageResX / 2) + moveCenter.x;
		approxCenter.x = (cfg->imageResY / 2) + moveCenter.y;
		if (cfg->verbosity >= 3)
		{
			stringstream ss;
			ss << "Centering frame: " << curImg->frameNo << " - move (x,y): (" << moveCenter.x << "," << moveCenter.y << ")";
			cfg->mMessages.lock();
			cfg->qMessages.push(ss.str());
			cfg->mMessages.unlock();
		}
		//calcNoiseCorners(curImg, cfg);

		//refine center with ray logic
		//moveCenter = rayCenter(approxCenter, curImg, 8, cfg);

		// crop image
		moveImage(cfg, curImg, moveCenter.x, moveCenter.y);

		/*firstAvgCount++;
		if (firstAvgCount >= cfg->averageLength)
		{
			//calculate exact center of jupiter and ellipse parameters
			firstAvg = true;
		}*/

		/* todo: ray centering algorithm
		//- center of mass only until the first average image is finished
		//- then calculate the exact center inside the average thread (see the comment there)
		//- from this center, go to the edge in 8 rays, checking for the brightest pixel and
		//stopping when reaching 30% brightness (use the average of 3-4 pixels)
		//- do some magic to calculate the move vector
		//- function prototype: raycenter(_in approx center x and y, _in imagedata, _in number of rays, _out exact center x and y, _out radius or ellipse parameters)
		*/

		// save the image for debugging
		//if (cfg->verbosity >= 3)
		//	debugPng(curImg->fileName, "_centered.png", curImg->rawBitmap);

		// send the image to the next thread
		cfg->mAverage.lock();
		cfg->qAverage.push(curImg);
		cfg->statQlen3++;
		cfg->mAverage.unlock();
		curImg = NULL;
	}

	// propagate shutdown to the next thread
	cfg->shutdownThread++;
}
