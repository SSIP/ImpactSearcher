#include "libimse.h"
#include "image_helper.h"


LIB_PUBLIC void impactSearcherStart(config *cfg) {
	// spawn worker threads, then return to caller
	thread(ioThread, cfg).detach();
}

LIB_PUBLIC void impactSearcherStop(config *cfg) {
	// signal the threads to end
	cfg->shutdownThread = 1;
}

/* Set a standard configuration
 *
 * Param *cfg is the reference to the configuration struct
 */
void loadDefaultConfig(config* cfg) {
	// set resolution to 0 if unknown
	cfg->imageResX = 0;
	cfg->imageResY = 0;
	cfg->keepFrames = KEEP_ALL;
	cfg->centerSkipPixels = 5;
	cfg->centerAlgo = CENTER_OF_MASS;
	cfg->centerThreshold = 50;
	cfg->averageLength = 10;
	cfg->checkSNR = 5.0;
	cfg->checkRadius = 1.0;
	cfg->verbosity = 1;
	cfg->rayBrightnessThreshold = 0.3;
	cfg->maxDiameter = 0.8;
	cfg->leadingAverageLength = 5;
	cfg->trailingAverageLength = 20;
	cfg->framebufferLength = 20;
}