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
