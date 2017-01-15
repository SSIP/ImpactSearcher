#include "definitions.h"
#include "image_helper.h"

int main(int argc, char **argv) {
	return 0;
}

struct impactSearcher {
	impactSearcher(config *stdConf);
	~impactSearcher();

	config *cfg;
};

impactSearcher::impactSearcher(config *stdConf) {
	cfg = stdConf;
	// spawn worker threads, then return to caller
	thread(ioThread, cfg);
	thread(centerThread, cfg);
	thread(averageThread, cfg);
	thread(presortThread, cfg);
	thread(checkThread, cfg);
}

impactSearcher::~impactSearcher() {
	// signal the threads to end
	cfg->shutdownThread = 1;
}

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

	for (y = 0; y < cfg->imageResY; y += cfg->centerSkipPixels) { // loop y axis
		for (x = 0; x < cfg->imageResX; x += cfg->centerSkipPixels) { // loop x axis
			pixel = frame->rawBitmap[x + y*  cfg->imageResX];
			if (pixel >= cfg->centerThreshold) {
				sumX += x*  pixel;
				sumY += y*  pixel;
				sumTotal += pixel;
			}
		}
	}
	move.x = -(int32_t)((sumX / sumTotal) - cfg->imageResX / 2);
	move.y = (sumY / sumTotal) - cfg->imageResY / 2;
	if (cfg->verbosity >= 3)
		fwprintf(stderr, (wstring(frame->fileName) + L": moveX=" + to_wstring(move.x) + L", moveY=" + to_wstring(move.y) + L"\n").c_str());
	return move;
}

void calculateReferenceCoordinates(){

}
