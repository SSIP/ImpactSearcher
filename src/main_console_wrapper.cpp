#include <conio.h>
#include "definitions.h"

void printHelp() {
	printf("Arguments:\
		   -s | -source        source folder searched for new images\n\
		   -d | -destination   destination folder where images are archived\n\
		   -k | -keep          archive [all], [int32_t]eresting or [can]didate frames\n\
		   -a | -averagesize   number of frames used for averaging\n\
		   -t | -threshold     min pixel intensity after subtraction\n\
		   -f | -filter        average filter size in parts of image\n\
		   -n | -snr           signal-to-noise ratio to qualify as candidate\n\
		   -r | -radius        radius to qualify as candidate\n");
}

config::config() {
	this->keepFrames = KEEP_NONE;
	this->archiveType = ARCHIVE_NONE;
	this->imageResX = this->imageResY = 0;
	this->centerSkipPixels = 0;
	this->centerAlgo = CENTER_NONE;
	this->averageLength = 0;
	//this->averageFilterSize = 0;
	this->checkSNR = this->checkRadius = 0;
	this->avgCrit = 1.5;
	this->devCrit = 1.5;
	this->frameAverage = NULL;
	this->statFramesIO = this->statQlen2 = this->statQlen3 = this->statQlen4 = this->statQlen5 = this->statInteresting = this->statCandidates = 0;
	this->shutdownThread = 0;
}

void setStdConf(config* cfg) {
	//cfg->imageResX = cfg->imageResY = 480;
	cfg->srcPath = "C:\\source";
	cfg->dstPath = "C:\\destination";
	cfg->keepFrames = KEEP_ALL;
	//cfg->archiveType = ARCHIVE_PNG; //todo: test bmp and png values
	cfg->centerSkipPixels = 5;
	cfg->centerAlgo = CENTER_OF_MASS;
	cfg->centerThreshold = 50;
	cfg->averageLength = 10;
	//cfg->averageFilterSize = 0.125;
	cfg->avgCrit = 1.5;
	cfg->devCrit = 1.5;
	cfg->checkSNR = 5.0;
	cfg->checkRadius = 1.0;
	cfg->verbosity = 2;
	cfg->rayBrightnessThreshold = 0.3;
}

int main(int argc, const char** argv) {
	// call to test code, comment out if not needed
	//test1();
	//return 0;

	// do help output if no arguments given
	//if(argc <= 1) {
	//	printHelp();
	//	return 0;
	//}

	// setup
	consoleInit();

	// set standard config, can be overridden by command line arguments
	config cfg;
	g_cfg = &cfg;
	setStdConf(&cfg);

	// read all arguments and set the config values
#define argParseBol(shortArg, longArg, dstVar) if(_stricmp(argv[i], longArg) == 0 || _stricmp(argv[i], shortArg) == 0) dstVar = true;
#define argParseStr(shortArg, longArg, dstVar) if(_stricmp(argv[i], longArg) == 0 || _stricmp(argv[i], shortArg) == 0) dstVar = argv[i + 1]
#define argParseInt(shortArg, longArg, dstVar) if(_stricmp(argv[i], longArg) == 0 || _stricmp(argv[i], shortArg) == 0) dstVar = atoi(argv[i + 1]);
#define argParseDbl(shortArg, longArg, dstVar) if(_stricmp(argv[i], longArg) == 0 || _stricmp(argv[i], shortArg) == 0) dstVar = atof(argv[i + 1]);
#define argParseFlt(shortArg, longArg, dstVar) if(_stricmp(argv[i], longArg) == 0 || _stricmp(argv[i], shortArg) == 0) dstVar = (float) atof(argv[i + 1]);

	for (int i = 1; i < argc; i += 2) {
		argParseStr("-s", "-source", cfg.srcPath);
		argParseStr("-d", "-destination", cfg.srcPath);
		//argParseInt("-a", "-averagesize", cfg.averageLength);
		//argParseFlt("-f", "-filter", cfg.averageFilterSize);
		argParseFlt("-n", "-snr", cfg.checkSNR);
		argParseFlt("-r", "-radius", cfg.checkRadius);
		if (_stricmp(argv[i], "-keep") == 0 || _stricmp(argv[i], "-k") == 0) {
			if (_stricmp(argv[i + 1], "all") == 0)
				cfg.keepFrames = KEEP_ALL;
			else if (_stricmp(argv[i + 1], "int32_t") == 0)
				cfg.keepFrames = KEEP_INTERESTING;
			else if (_stricmp(argv[i + 1], "can") == 0)
				cfg.keepFrames = KEEP_CANDIDATE;
			else
				throw invalid_argument(NULL);
		}
	}

	// todo: call constructor, then start()

	// statistics output format; standard console size is 80x25 chars
	char output[] = "\
					====================== Solar System Impact Project (SSIP) ======================\n\
					= Statistic = Input images: %9u | Interesting: %6u | Candidates: %4u =\n\
					= counters: = Queue length: Center %3u | Average %3u | Presort %3u | Check %3u =\n\
					================================================================================\n\
					working...";
	// do statistics output
	for (;;) {
		consoleErase();
		// statistics output
		printf(output, cfg.statFramesIO, cfg.statInteresting, cfg.statCandidates, cfg.statQlen2, cfg.statQlen3, cfg.statQlen4, cfg.statQlen5);
		// if signal for graceful shutdown was received:
		if (consolePeek() == 'q') { // todo: doesn't work
			// todo: call stop()
			break;
		}
		Sleep(1000);
	}
	// wait for the last thread to finish
	while (cfg.shutdownThread != 6)
		Sleep(100);
	printf("\nShutdown finished");
	_getch();
}
