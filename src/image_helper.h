#pragma once
#include "libimse.h"

void initImageParameters(const uint32_t width, const uint32_t height);

/*
*	variable length byte storage class
*/

class byteStorage {
	uint8_t* data;
	uint32_t allocatedLength, currentLength;

public:
	byteStorage() : data{ NULL }, allocatedLength{ 0 }, currentLength{ 0 } {};
	~byteStorage() { if (data) delete data; };
	void set(uint8_t* data, uint32_t length);
	void append(uint8_t* data, uint32_t length);
	void clear() { currentLength = 0; };
	uint32_t getLength() { return currentLength; };
	uint8_t* getData() { return this->data; };
	void reserve(uint32_t length);
};

/*
* 	TAR archiving
*/

union tarHeader {
	struct {
		uint8_t name[100];
		uint8_t mode[8];
		uint8_t uid[8];
		uint8_t gid[8];
		uint8_t size[12];
		uint8_t mtime[12];
		uint8_t checksum[8];
		uint8_t linkflag[1];
		uint8_t linkname[100];
		uint8_t pad[255];
	};
	uint8_t rawHeader[512];
};

class tarFile {
	FILE* fOut;
	tarHeader header;
	uint8_t padding[512];
	vector<uint8_t*> pngBuffer;

public:
	tarFile();
	~tarFile() {};
	void open(char* fileName);
	void close();
	void addFileFromMem(char* fileName, unsigned long long fileSize, unsigned long long fileTime, uint8_t* fileData);
	void addFileFromFs(char* fileName, unsigned long long fileTime, char* sourceFileName);

private:
	void setHeader(char* fileName, unsigned long long fileSize, unsigned long long fileTime);
};
