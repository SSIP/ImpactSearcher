#include "libimse.h"
#include "image_bmp.h"
#include "filesystem.h"
#include <sstream>

void ioThread(config* cfg) {
	image* curImg;
	uint32_t frameNo = 0;

	for (; cfg->shutdownThread != 1; this_thread::sleep_for(chrono::milliseconds(10))) {

		auto files = getFiles(cfg->srcPath, ".bmp");

		//TODO:
		// - detect framerate and set config->framerate, refresh from time to time. a roug estimation has to be given as command line or config file parameter for the first average image.

		// work through all new files
		for(auto it = files.begin(); it != files.end(); it++) {
			frameNo++;
			if (cfg->verbosity >= 2)
			{
				stringstream ss;
				ss << "Found " << it->name;
				cfg->mMessages.lock();
				cfg->qMessages.push(ss.str());
				cfg->mMessages.unlock();
			}
			// read the source bitmap
			auto inputData = bmp_read(it->name, it->size, cfg->imageResX, cfg->imageResY);
			
			// emplace the input data into a new image struct
			curImg = new image(cfg->imageResX, cfg->imageResY, inputData, frameNo);
			curImg->fileName = it->name;

			// update statistics
			cfg->statFramesIO++;

			// send the image to the next thread
			cfg->mCenter.lock();
			cfg->qCenter.push(curImg);
			cfg->statQlen2++;
			cfg->mCenter.unlock();
			curImg = NULL;
		}

		// delete input files and empty the queue
		deleteFiles(files);
		files.clear();
	}

	// propagate shutdown to the next thread
	cfg->shutdownThread++;
}
