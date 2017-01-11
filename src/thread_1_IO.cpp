#include "definitions.h"
#include "image_bmp.h"
#include "cslib/FsTools.h"

string getTimeDateString() {
	char buffer[20];

	time_t rawtime = time(NULL);
	tm* timeinfo = localtime(&rawtime);

	if(strftime(buffer, 20, "%Y-%m-%d_%H-%M", timeinfo) == NULL)
		throw runtime_error(NULL);

	return string(buffer);
}

void initImageParameters(const uint32_t width, const uint32_t height) {
	extern config* g_cfg;
	g_cfg->imageResX = width;
	g_cfg->imageResY = height;
	g_cfg->leadingAverage = new averageImage(width, height, g_cfg->leadingAverageLength);
	g_cfg->trailingAverage = new averageImage(width, height, g_cfg->trailingAverageLength);

}
// my filesystem tool functions are unicode capable but this is not implemented here
// if unicode is needed, upgrading to std::u16string is advised
void ioThread(config* cfg) {
	fsTools fs;
	image* curImg;

	fs.setBaseDir((char*)cfg->srcPath.c_str());

	for (; cfg->shutdownThread != 1; this_thread::sleep_for(chrono::milliseconds(10))) {

		//TODO:
		// - detect framerate and set config->framerate, refresh from time to time. a roug estimation has to be given as command line or config file parameter for the first average image.

		// get all files in the input directory
		fs.getFilesByType(".bmp");

		// work through all new files
		for(list<fileInf*>::iterator it = fs.files.begin(); it != fs.files.end(); it++) {
			// read the source bitmap
			auto inputData = bmp_read((*it)->name, (*it)->size, cfg->imageResX, cfg->imageResY);
			
			// emplace the input data into a new image struct
			curImg = new image(cfg->imageResX, cfg->imageResY, inputData);
			curImg->fileName = (*it)->name;

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
		fs.deleteIndexedFiles();
		fs.files.clear();
	}

	// propagate shutdown to the next thread
	cfg->shutdownThread++;
}

/* old code including archiving of images. needs some care. maybe use subdirectories instead of tar archives
#include "image_png.h"
#include "image_helper.h"
void ioThread(config* cfg) {
	fsTools fs;
	image* curImg;
	uint8_t* curData;
	byteStorage pngData;
	bmpFile bmp;
	tarFile tar;
	time_t lastTarOpen = 0;

	fs.setBaseDir((int8_t*)cfg->srcPath.c_str());

	for(;;) {

		//TODO:
		// - detect framerate and set config->framerate, refresh from time to time. a roug estimation has to be given as command line or config file parameter for the first average image.

		// check if we need to open a new tar archive or delete old ones
		if (cfg->archiveType != ARCHIVE_NONE && time(NULL) > lastTarOpen + 10*  60*  1000) { // 10 minutes
			tar.close();
			tar.open((int8_t*) (cfg->dstPath + "//" + getTimeDateString() + ".tar").c_str());
			lastTarOpen = time(NULL);

			// todo: delete tar archives older than x*10 minutes (-> make a list of the filenames first)
		}

		// get all files in the input directory
		fs.getFilesByType(".bmp");

		// work through all new files
		for(list<fileInf*>::iterator it = fs.files.begin(); it != fs.files.end(); it++) {
			// read the source bitmap
			bmp.read((*it)->name, &curData);

			if (cfg->archiveType != ARCHIVE_NONE) {
				// create an archiving filename
				string outName((*it)->name);
				outName.erase(0, outName.find_last_of("\\/") + 1);
				outName.erase(outName.find('.'));
				if (cfg->archiveType == ARCHIVE_PNG) {
					outName.append(".png");
					// write the png data and add it to the tar archive
					createPng(cfg->imageResX, cfg->imageResY, 8, true, &curData, &pngData);
					tar.addFileFromMem((int8_t*)outName.c_str(), pngData.getLength(), (uint32_t)(*it)->creation, pngData.getData());
					pngData.clear();
				}
				else if (cfg->archiveType == ARCHIVE_BMP) {
					outName.append(".bmp");
					// add the bmp to the tar archive
					tar.addFileFromFs((int8_t*)outName.c_str(), (uint32_t)(*it)->creation, (*it)->name);
				}
				else
					throw invalid_argument(NULL);
			}

			// move the data into the image struct
			curImg = new image(cfg->imageResX, cfg->imageResY, curData);
			strcpy_s(curImg->fileName, sizeof curImg->fileName, (*it)->name);

			// update statistics
			cfg->statFramesIO++;

			// and send the image to the next thread
			cfg->mCenter.lock();
			cfg->qCenter.push(curImg);
			cfg->statQlen2++;
			cfg->mCenter.unlock();
			curImg = NULL;
		}

		// delete input files and empty the queue
		fs.deleteIndexedFiles();
		fs.files.clear();

		// if shutdown signal was received, we finish this thread here
		if(cfg->shutdownInit) {
			tar.close();
			cfg->shutdownIO = true;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(10));
	}
}
*/