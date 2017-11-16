#include "libimse.h"
#include "image_helper.h"
#include <iostream>

// todo: do we need the average image outside of this thread -> no

/* todo: every 10 minutes, determine the center and edge of the planet.
	start with center of mass, then 32 rays to the edge until 30% brightness is reached
	then fit an ellipse
*/

void averageThread(config* cfg) {
	image* curImg = NULL;
	queue<image*> qBuf1, qBuf2, qBuf3;
	uint32_t len1 = 0, len2 = 0, len3 = 0; // length of leading average, frame buffer, trailing average

	cfg->leadingAverage = new averageImage(cfg->imageResX, cfg->imageResY, cfg->leadingAverageLength);
	cfg->trailingAverage = new averageImage(cfg->imageResX, cfg->imageResY, cfg->trailingAverageLength);

	for (; cfg->shutdownThread != 3; this_thread::sleep_for(chrono::milliseconds(10))) {
		// wait for the ui
		cfg->mUiCenter.lock();
		cfg->mUiCenter.unlock();
		// get the next image from our queue
		cfg->mAverage.lock();

		if(cfg->qAverage.empty()) {
			cfg->mAverage.unlock();
			continue;
		}
		else {
			curImg = cfg->qAverage.front();
			cfg->qAverage.pop();
			cfg->statQlen3--;
			cfg->mAverage.unlock();
		}

		// curImg now contains the current image ready for averaging

		/* this algorithm is to reduce noise in the subtracted image
			todo: test this algo and compare it to the above one
		*/
		// add delta (timediff) of frames between leading and trailing average (in config)
		// update first average and save the image
		cfg->leadingAverage->shuffle(curImg->rawBitmap);
		qBuf1.push(curImg);
		curImg = NULL;

		// if the first average has not yet reached its length, loop
		if (++len1 <= cfg->leadingAverageLength)
			continue;

		// the first average is now full, fill the intermediate frame buffer
		if (qBuf1.empty())
			throw runtime_error(NULL);
		qBuf2.push(qBuf1.front());
		qBuf1.pop();

		// if the intermediate frame buffer has not yet reached its length, loop
		if (++len2 <= cfg->framebufferLength)
			continue;

		// the intermediate frame buffer is now full, update the second average and save the image
		if (qBuf2.empty())
			throw runtime_error(NULL);
		cfg->trailingAverage->shuffle(qBuf2.front()->rawBitmap);
		qBuf3.push(qBuf2.front());
		qBuf2.pop();

		// if the second average has not yet reached its length, loop
		if (++len3 <= cfg->trailingAverageLength)
			continue;

		// all frame buffers are full now, so create the subtracted image and forward the images to the next thread
		// todo (important): to which image should the diff be associated?
		curImg = qBuf3.front();
		qBuf3.pop();
		uint32_t pixelSum = 0;
		for (uint32_t i = 0; i < (cfg->imageResX*  cfg->imageResY); i++)
			pixelSum += curImg->diffBitmap[i] = cfg->leadingAverage->currentAverage[i] - cfg->trailingAverage->currentAverage[i];
		//curImg->avgValue = pixelSum / (cfg->imageResX*  cfg->imageResY);

		// send the image to the next thread
		cfg->mPresort.lock();
		cfg->qPresort.push(curImg);
		cfg->statQlen4++;
		cfg->mPresort.unlock();
		curImg = NULL;
	}

	// propagate shutdown to the next thread
	cfg->shutdownThread++;
}

averageImage::averageImage(uint32_t imageResX, uint32_t imageResY, uint32_t length) {
	this->imageSize = imageResX*  imageResY;

	this->currentAverage = new uint8_t[this->imageSize];
	memset(this->currentAverage, 0, this->imageSize);
	this->sum = new uint32_t[this->imageSize];
	memset(this->sum, 0, this->imageSize*  sizeof(int32_t));

	this->summands.reserve(length);
	uint8_t* mem;
	for (uint32_t i = 0; i < length; i++) {
		mem = new uint8_t[this->imageSize];
		this->summands.push_back(mem);
		memset(this->summands[i], 0, this->imageSize);
	}

	this->summandsPos = 0;
	this->summandsSize = 0;
	this->summandsPosOld = 1;
}

// todo: remove code for semi full buffers because now we wait until they are full until we use the data
void averageImage::shuffle(uint8_t* rawBitmapInput) {
	cout << this->summandsPos << endl;
	cout << this->summandsPosOld << endl;
	uint8_t* newImage = this->summands[this->summandsPos],* oldImage = this->summands[this->summandsPosOld];
	// copy new image into local storage
	memcpy(newImage, rawBitmapInput, this->imageSize);
	// add new image to and delete old image from sum
	if (this->summandsSize < this->summands.size()) {
		for (uint32_t i = 0; i < this->imageSize; i++)
			this->sum[i] = this->sum[i] + newImage[i];
		this->summandsSize++;
	}
	else {
		for (uint32_t i = 0; i < this->imageSize; i++)
			this->sum[i] = this->sum[i] + newImage[i] - oldImage[i];
	}

	// update average
	for (uint32_t i = 0; i < this->imageSize; i++)
		this->currentAverage[i] = (int32_t)floor((float)this->sum[i] / summandsSize + 0.5);
	// progress position
	//if (this->summandsSize < this->summands.size())
		
	this->summandsPosOld = (this->summandsPosOld + 1) % this->summands.size();
	this->summandsPos = (this->summandsPos + 1) % this->summands.size();
}
