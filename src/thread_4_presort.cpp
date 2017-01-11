#include "definitions.h"
#include "image_helper.h"

void presortThread(config* cfg) {
	image* curImg = NULL;

	for (; cfg->shutdownThread != 4; this_thread::sleep_for(chrono::milliseconds(10))) {
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
