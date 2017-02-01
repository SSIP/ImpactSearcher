#include "image_bmp.h"

// align memory to 16 bit for direct pointer access
#pragma pack(push, 2)

// source: windows 8.1 driver kit WinGDI.h line 933
struct bitmapFileHeader {
	uint16_t	bfType;
	uint32_t	bfSize;
	uint16_t	bfReserved1;
	uint16_t	bfReserved2;
	uint32_t	bfOffBits;
};

// source: windows 8.1 driver kit WinGDI.h line 808
struct bitmapInfoHeader {
	uint32_t	biSize;
	int32_t				biWidth;
	int32_t				biHeight;
	uint16_t	biPlanes;
	uint16_t	biBitCount;
	uint32_t	biCompression;
	uint32_t	biSizeImage;
	int32_t				biXPelsPerMeter;
	int32_t				biYPelsPerMeter;
	uint32_t	biClrUsed;
	uint32_t	biClrImportant;
};

// restore memory alignment to original value
#pragma pack(pop)

// Code loosely inspired by http://tipsandtricks.runicsoft.com/Cpp/BitmapTutorial.html
uint8_t* bmp_read(const string fileName, const uint64_t fileSize, const uint32_t width, const uint32_t height) {
	// open output file
	FILE* fIn;
	if ((fIn = fopen(fileName.c_str(), "rb")) == 0)
		throw runtime_error(NULL);

	// allocate memory for the whole file and read it
	uint8_t* rawBuffer = new uint8_t[fileSize];
	if (fread(rawBuffer, 1, fileSize, fIn) != fileSize)
		throw runtime_error(NULL);

	// close the file
	if (fclose(fIn) != 0)
		throw runtime_error(NULL);

	// set the header structures to their appropriate positions (needs memory alignment as stated above)
	bitmapFileHeader* file_header = (bitmapFileHeader*)rawBuffer;
	bitmapInfoHeader* info_header = (bitmapInfoHeader*)(rawBuffer + sizeof(bitmapFileHeader));

	// set the image data starting position
	uint8_t* imgBuffer = rawBuffer + file_header->bfOffBits;

	// check if the image corresponds to our needs
	// check "BM" magic bytes (little endian)
	if (file_header->bfType != 'MB')
		throw runtime_error(NULL);
	// if sizes don't match, the file is corrupted
	if (file_header->bfSize != fileSize)
		throw runtime_error(NULL);
	// check if uncompressed (we don't handle compressed images)
	if (info_header->biCompression != 0) // BI_RGB = 0
		throw runtime_error(NULL);

	// if this is the first image, call initializer
	if (width == 0 || height == 0)
		initImageParameters(info_header->biWidth, info_header->biHeight);

	// otherwise, check width and height
	else if (info_header->biWidth != width || info_header->biHeight != height)
		throw runtime_error(NULL);

	// image seems ok, create the output buffer (save the pointer locally in case of an abortion)
	// this is the actual image buffer that will be used during the full analysation process
	uint8_t* outBuffer = new uint8_t[info_header->biWidth*  info_header->biHeight];

	// determine amount of padding bytes
	uint32_t lineLength = 0, srcLength = info_header->biWidth*  info_header->biBitCount / 8;
	while ((srcLength + lineLength) % 4 != 0)
		lineLength++;
	lineLength += info_header->biWidth;

	// convert the bmp bytes to our internal image format
	// switch by amount of bits per pixel (8=grayscale, 24=RGB)
	switch (info_header->biBitCount) {
	case 8:
		for (uint32_t x = 0, offset = 0; x < height; x++, offset += width)
			memcpy(outBuffer + offset, imgBuffer + (height - x - 1)*  lineLength, width);
		break;
	case 24:
		// do grayscale conversion
		// as of now, we only handle "fake color" images, where the values for R, G and B are equal (and result from an image three times in size of a real grayscale image)
		uint32_t pos;
		for (uint32_t y = 0; y < height; y++)
			for (uint32_t x = 0; x < width; x++) {
				pos = ((height - y - 1)*  lineLength + x)*  3;
				if (imgBuffer[pos] != imgBuffer[pos + 1] || imgBuffer[pos] != imgBuffer[pos + 2])
					throw runtime_error(NULL);
				outBuffer[y*  width + x] = imgBuffer[pos];
			}
		break;
	default:
		throw runtime_error(NULL);
	}

	// cleanup and return the local output pointer
	delete[] rawBuffer;
	return outBuffer;
}
