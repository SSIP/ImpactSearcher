#include "image_helper.h"
#include <stdio.h>
#include <sys/stat.h>

/*
*	some platform compatibility checks and other $foo
*/

#if defined(__i386__)
#define ARCH_OK
#elif defined(_M_IX86)
#define ARCH_OK
#elif defined(__amd64__)
#define ARCH_OK
#elif defined(_M_AMD64)
#define ARCH_OK
#endif
#ifndef ARCH_OK
#error Tested only for x86 architectures. If you are brave, remove this line.
#endif

/*
*	the main image class
*/

image::image(uint32_t imageResX, uint32_t imageResY, uint8_t* inputData) {
	uint32_t size = imageResX*  imageResY;

	this->rawBitmap = inputData;
	this->diffBitmap = new int16_t[size];
	memset(this->diffBitmap, 0, size);
	this->impacts = NULL;
	this->timestamp = 0;
	memset(this->diffHistogram, 0, 4*  512);
}

image::~image() {
	delete[] this->rawBitmap;
	delete[] this->diffBitmap;
	if (this->impacts) {
		this->impacts->clear();
		delete this->impacts;
	}
}

/*
*	variable length byte storage class
*/

void byteStorage::set(uint8_t* data, uint32_t length) {
	if (this->data == NULL) {
		// create a new buffer
		uint32_t newSize = (uint32_t)(length*  1.4);
		this->data = new uint8_t[newSize];
		memcpy(this->data, data, length);
		// update the structure
		this->currentLength = length;
		this->allocatedLength = newSize;
	}
	else {
		// use the existing buffer
		if (this->currentLength + length > this->allocatedLength) {
			// allocate a larger chunk
			uint32_t newSize = (uint32_t)(this->allocatedLength*  1.4);
			if (length > newSize)
				newSize = (uint32_t)(length*  1.4);
			uint8_t* newMem = new uint8_t[newSize];
			// copy old data and append new data
			memcpy(newMem, data, length);
			// update the structure
			delete this->data;
			this->data = newMem;
			this->currentLength = length;
			this->allocatedLength = newSize;
		}
		else {
			// data fits in current space
			memcpy(this->data, data, length);
			this->currentLength = length;
		}
	}
}

void byteStorage::append(uint8_t* data, uint32_t length) {
	if (this->data == NULL) {
		// create a new buffer
		uint32_t newSize = (uint32_t)(length*  1.4);
		this->data = new uint8_t[newSize];
		memcpy(this->data, data, length);
		// update the structure
		this->currentLength = length;
		this->allocatedLength = newSize;
	}
	else {
		// use the existing buffer
		if (this->currentLength + length > this->allocatedLength) {
			// allocate a larger chunk
			uint32_t newSize = (uint32_t)(this->allocatedLength*  1.4);
			if (this->currentLength + length > newSize)
				newSize = (uint32_t)((this->currentLength + length)*  1.4);
			uint8_t* newMem = new uint8_t[newSize];
			// copy old data and append new data
			memcpy(newMem, this->data, this->currentLength);
			memcpy(newMem + this->currentLength, data, length);
			// update the structure
			delete this->data;
			this->data = newMem;
			this->currentLength += length;
			this->allocatedLength = newSize;
		}
		else {
			// data fits in current space
			memcpy(this->data + this->currentLength, data, length);
			this->currentLength += length;
		}
	}
}

void byteStorage::reserve(uint32_t length) {
	if (this->data)
		delete this->data;
	this->data = new uint8_t[length];
}


/*
* 	TAR archiving
*/


tarFile::tarFile() {
	this->fOut = NULL;
	memset(this->header.rawHeader, 0, sizeof this->header.rawHeader);
	memset(this->padding, 0, sizeof this->padding);

	// set the file header's static values
	memcpy(this->header.mode, "100777 \0", 8);
	memcpy(this->header.uid,  "     0 \0", 8);
	memcpy(this->header.gid,  "     0 \0", 8);
	this->header.size[11] =		0x20;			// " "
	this->header.mtime[11] =	0x20;			// " "
	*this->header.linkflag =	0x30;			// "0"
}

void tarFile::open(char* fileName) {
	if((this->fOut = fopen(fileName, "wb")) == NULL)
		throw runtime_error(NULL);
}

void tarFile::close() {
	if(this->fOut) {
		// append 1024 zero bytes to signal EOF
		fwrite(this->padding, 1, 512, this->fOut);
		fwrite(this->padding, 1, 512, this->fOut);
	
		// close the file
		fclose(this->fOut);
		this->fOut = NULL;
	}
}

void tarFile::setHeader(char* fileName, unsigned long long fileSize, unsigned long long fileTime) {
	// max filesize for original tar is 8 GiB, this is enough for us
	if (fileSize >= 8589934591)
		throw out_of_range(NULL);
	// timestamp overflows 2242-03-16 12:56:31
	if (fileTime >= 8589934591)
		throw out_of_range(NULL);
	// max filename length for tar is 99 chars
	if (strlen(fileName) >= 100)
		throw invalid_argument(NULL);

	// set the header structure
	memset(this->header.name, 0, sizeof this->header.name);
	strcpy((char*) this->header.name, (const char*)fileName);
	SNPRINTF((char*) this->header.size, 11, "%11o", fileSize);
	SNPRINTF((char*) this->header.mtime, 11, "%11o", fileTime);
	memcpy(this->header.checksum, "        ", 8);

	// compute and set checksum
	uint32_t checksum = 0;
	for(uint32_t i = 0; i < 512; i++)
		checksum += this->header.rawHeader[i];
	SNPRINTF((char*) this->header.checksum, 6, "%6o", checksum);
	this->header.checksum[7] = 0;
}

void tarFile::addFileFromMem(char* fileName, unsigned long long fileSize, unsigned long long fileTime, uint8_t* fileData) {
	if (this->fOut) {
		this->setHeader(fileName, fileSize, fileTime);

		// append the header data, file data and padding bytes to the archive
		fwrite(this->header.rawHeader, 1, 512, this->fOut);
		fwrite(fileData, 1, fileSize, this->fOut);
		fwrite(this->padding, 1, 512 - (fileSize % 512), this->fOut);
	}
}

void tarFile::addFileFromFs(char* fileName, unsigned long long fileTime, char* sourceFileName) {
	if (this->fOut) {
		// get file size
		struct stat fileInfo;
		if (stat(fileName, &fileInfo) != 0)
			throw runtime_error(NULL);
		// open the file
		FILE* fIn;
		if ((fIn = fopen(fileName, "rb")) == NULL)
			throw runtime_error(NULL);
		// read it into memory
		int8_t* buf = new int8_t[fileInfo.st_size];
		if (fread(buf, 1, fileInfo.st_size, fIn) != fileInfo.st_size) {
			delete buf;
			throw runtime_error(NULL);
		}
		fclose(fIn);

		this->setHeader(fileName, fileInfo.st_size, fileTime);

		// append the header data, file data and padding bytes to the archive
		fwrite(this->header.rawHeader, 1, 512, this->fOut);
		fwrite(buf, 1, fileInfo.st_size, this->fOut);
		fwrite(this->padding, 1, 512 - (fileInfo.st_size % 512), this->fOut);
	}
}
