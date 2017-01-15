#include "definitions.h"
#include "image_bmp.h"
#include "filesystem.h"

void initImageParameters(const uint32_t width, const uint32_t height) {
	extern config* g_cfg;
	g_cfg->imageResX = width;
	g_cfg->imageResY = height;
	g_cfg->leadingAverage = new averageImage(width, height, g_cfg->leadingAverageLength);
	g_cfg->trailingAverage = new averageImage(width, height, g_cfg->trailingAverageLength);
}

void ioThread(config* cfg) {
	image* curImg;

	for (; cfg->shutdownThread != 1; this_thread::sleep_for(chrono::milliseconds(10))) {

		auto files = getFiles(cfg->srcPath, L".bmp");

		//TODO:
		// - detect framerate and set config->framerate, refresh from time to time. a roug estimation has to be given as command line or config file parameter for the first average image.

		// work through all new files
		for(auto it = files.begin(); it != files.end(); it++) {
			// read the source bitmap
			auto inputData = bmp_read(it->name, it->size, cfg->imageResX, cfg->imageResY);
			
			// emplace the input data into a new image struct
			curImg = new image(cfg->imageResX, cfg->imageResY, inputData);
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
