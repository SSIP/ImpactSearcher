#include "libimse.h"
#include "image_helper.h"


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

