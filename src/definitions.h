#pragma once
#include <string>
#include <forward_list>
#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdint>
using namespace std;

enum enumKeepFrames {
	KEEP_NONE,
	KEEP_ALL,
	KEEP_INTERESTING,
	KEEP_CANDIDATE
};

enum enumArchiveType {
	ARCHIVE_NONE,
	ARCHIVE_BMP,
	ARCHIVE_PNG
};

enum enumCenterAlgorithm {
	CENTER_NONE,
	CENTER_OF_MASS
};

struct coordinates {
	uint32_t x, y;
};

struct direction {
	double_t x, y;
};

struct impact {
	coordinates center;
	uint32_t maxVal, radius, SNR, totalLum;
	vector<coordinates> ROI; //Region of Interest
};

struct image {
	string fileName;
	uint8_t* rawBitmap;
	int16_t* diffBitmap;
	uint8_t avgValue;
	double varianceValue, stdDev;
	forward_list<impact>* impacts;
	time_t timestamp;
	uint32_t diffHistogram[512];
	uint32_t interestingStartValue;

	// fit of circle around planet. image is centered on planet center.
	uint32_t radiusPlanet;
	direction anglePlanet;

	image(uint32_t imageResX, uint32_t imageResY, uint8_t* inputData);
	~image();
};

struct averageImage {
	uint8_t* currentAverage;
	uint32_t* sum;
	vector<uint8_t*> summands;
	uint32_t summandsPos, summandsPosOld, summandsSize, imageSize;

	averageImage(uint32_t imageResX, uint32_t imageResY, uint32_t length);
	void shuffle(uint8_t* rawBitmapInput);
};

struct config {
	// command line/config file arguments
	// 01: IO thread
	string srcPath, dstPath;			// input & output folder
	enumKeepFrames keepFrames;			// archive all, interesting or candidate frames
	enumArchiveType archiveType;		// archive as bmp or png files (or not at all)
	uint32_t imageResX, imageResY;		// image resolution in pixels
	// 02: center thread
	uint32_t centerSkipPixels;			// = 5; skip pixels for centering algorithm
	enumCenterAlgorithm centerAlgo;		// = CENTER_OF_MASS
	uint32_t centerThreshold;			// = ignore pixels below this brightness for centering
	// 03: average thread
	uint32_t averageLength;				// = 10; number of images for average calculation. 1/2 sec, max. 30, min 8
	uint32_t leadingAverageLength;		// = 4; todo: determine all average values by frame rate! min. 3, target: 1/5 sec |||||NOT LONGER REQUIRED|||||
	uint32_t trailingAverageLength;		// = ??? todo: sven?
	uint32_t framebufferLength;			// = 10; images to wait between leading and trailing average. min. 10, target: 1/2 sec|||||BUFFERING 0.5 seconds|||||
	//double averageFilterSize;			// = 0.125; average filter size in parts of image
	// 04: presort thread
	float avgCrit, devCrit;				// = 1.50; multiplicator for critical value above expected value in presort
	// 05: check thread
	double checkSNR;					// = 5 signal-to-noise ratio to qualify as candidate
	double checkRadius;					// = 1.00 radius to qualify as candidate
	int32_t verbosity;						// = 1; standard, 2 = more, 3 = max, 0 = none
	float framerate;					// framerate of incoming images
	float rayBrightnessThreshold;		// = 0.3 limb darkening threshold 

	// data storage
	averageImage* frameAverage;	// average image structure
	averageImage* leadingAverage,* trailingAverage;
	// statistic counters
	uint32_t statFramesIO, statQlen2, statQlen3, statQlen4, statQlen5, statInteresting, statCandidates;

	// thread synchronization and data passing variables
	queue<image*> qCenter, qAverage, qPresort, qCheck;
	mutex mCenter, mAverage, mPresort, mCheck;
	uint32_t shutdownThread;

	config();
};

void ioThread(config* cfg);
void centerThread(config* cfg);
void averageThread(config* cfg);
void presortThread(config* cfg);
void checkThread(config* cfg);
coordinates massCenter(image* frame, config* cfg);
coordinates rayCenter(coordinates approximateCenter, image* frame, int32_t numberRays, config* cfg);
