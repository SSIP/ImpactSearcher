#include "libimse.h"
#include "image_helper.h"


LIB_PUBLIC void impactSearcherStart(config *cfg) {
	// spawn worker threads, then return to caller
	thread(ioThread, cfg).detach();
	thread(centerThread, cfg).detach();
	thread(averageThread, cfg).detach();
	thread(presortThread, cfg).detach();
	thread(checkThread, cfg).detach();
}

LIB_PUBLIC void impactSearcherStop(config *cfg) {
	// signal the threads to end
	cfg->shutdownThread = 1;
}

