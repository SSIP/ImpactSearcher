#pragma once
#include <forward_list>
#include <list>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdint>
#include <memory.h>
#include <string>

// set library export macros
#if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
#ifdef BUILDING_LIB
	#define LIB_PUBLIC __declspec(dllexport)
#else
	#define LIB_PUBLIC __declspec(dllimport)
#endif
#define LIB_LOCAL
#else
#if __GNUC__ >= 4
	#define LIB_PUBLIC __attribute__ ((visibility ("default")))
	#define LIB_LOCAL  __attribute__ ((visibility ("hidden")))
#else
	#define LIB_PUBLIC
	#define LIB_LOCAL
#endif
#endif

using namespace std;

#if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

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

struct deltacoords {
	int32_t x, y;
};

struct direction {
	double_t x, y;
};

struct impact {
	coordinates center;
	uint32_t maxVal, radius, SNR, totalLum;
	vector<coordinates> ROI; //Region of Interest
};

struct noise {
	double average, variance, stdDev;
	uint32_t sampleSize;
};

struct image {

/*
 * image[x][y]
 *  0 -------------------------------------------------------> X axis
 *  |  top left                                     top right
 *  |    0:0   0:1   0:2   0:3 …   0:97   0:98   0:99   0:100
 *  |    1:0   1:1   1:2       …          1:98   1:99   1:100
 *  |    2:0   2:1             …                 2:99   2:100
 *  |    3:0                   …                        3:100
 *  |     …     …     …     …  …    …      …      …      …
 *  |   97:0                   …                        7:100
 *  |   98:0  98:1             …                98:99  98:100
 *  |   99:0  99:1  99:2       …	     99:98  99:99  99:100
 *  |  100:0 100:1 100:2 100:3 … 100:97 100:98 100:99 100:100
 *  |  bottom left                               bottom right
 *  v
 * 
 *  Y axis
 */
	uint32_t frameNo;
	string fileName;
	uint8_t* rawBitmap;
	int16_t* diffBitmap;
	noise imgNoise;
	forward_list<impact>* impacts;
	time_t timestamp;
	uint32_t diffHistogram[512];
	uint32_t interestingStartValue;

	// fit of circle around planet. image is centered on planet center.
	uint32_t radiusPlanet;
	direction anglePlanet;

	image(uint32_t imageResX, uint32_t imageResY, uint8_t* inputData, uint32_t frameNo);
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
	int32_t verbosity;					// = 1; standard, 2 = more, 3 = all, 4 = debug, may change result, 0 = none
	float framerate;					// framerate of incoming images
	float rayBrightnessThreshold;		// = 0.3 limb darkening threshold 

	// data storage
	averageImage* leadingAverage,* trailingAverage;
	// statistic counters
	uint32_t statFramesIO, statQlen2, statQlen3, statQlen4, statQlen5, statInteresting, statCandidates;

	// thread synchronization and data passing variables
	queue<image*> qCenter, qAverage, qPresort, qCheck;
	mutex mCenter, mAverage, mPresort, mCheck;

	// the UI mutexes concern the follow up queue of the coresponding thread. For example, the GUI wants to display the already centered image.
	// To make sure that it can get an image from the queue between centering and averaging, it will acquire the mUiCenter mutex and then get an
	// image without popping it from the queue. During the time the mutex is held by the UI, the average thread is not allowed to remove an image.
	mutex mUiCenter, mUiAverage, mUiPresort, mUiCheck;

	// Messages to be displayed on the user interface
	queue<string> qMessages;
	mutex mMessages;

	uint32_t shutdownThread;

	// ratio of planet size to image width or height, whichever is smaller
	double maxDiameter;
	// Length of triangle leg in corners
	uint32_t cornerTriLeg, numCornerPixels;
};

void ioThread(config* cfg);
void centerThread(config* cfg);
void averageThread(config* cfg);
void presortThread(config* cfg);
void checkThread(config* cfg);

LIB_PUBLIC void impactSearcherStart(config *cfg);
LIB_PUBLIC void impactSearcherStop(config *cfg);