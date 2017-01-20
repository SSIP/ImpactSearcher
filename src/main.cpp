#include "definitions.h"
#include "image_helper.h"

class something
{
}

uint32_t asdf;

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

LIB_PUBLIC void impactSearcherStart(config *cfg) {
	// spawn worker threads, then return to caller
	thread(ioThread, cfg);
	thread(centerThread, cfg);
	thread(averageThread, cfg);
	thread(presortThread, cfg);
	thread(checkThread, cfg);
}

LIB_PUBLIC void impactSearcherStop(config *cfg) {
	// signal the threads to end
	cfg->shutdownThread = 1;
}

