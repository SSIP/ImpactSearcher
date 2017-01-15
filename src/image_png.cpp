#include "image_png.h"
#include "png.h"
#include "pngstruct.h"

void myPngWriteFunction(png_structp png_ptr, png_bytep data, png_size_t length) {
	((byteStorage*)png_ptr->io_ptr)->append(data, length);
}

void myPngFlushFunction(png_structp png_ptr) {
	// nothing to do here
}

void createPng(const uint32_t width, const uint32_t height, const uint8_t bitDepth, const bool grayscale, const void* imageMatrix, byteStorage* outputData) {
	// sanitize inputs
	if (width == 0 || width > 4094)
		throw out_of_range(NULL);
	if (height == 0 || height > 4096)
		throw out_of_range(NULL);
	if (bitDepth != 8 && bitDepth != 16)
		throw out_of_range(NULL);
	if (imageMatrix == NULL || outputData == NULL)
		throw invalid_argument(NULL);

	// Create and initialize the png_struct with the desired error handler
	// functions.  If you want to use the default stderr and longjump method,
	// you can supply NULL for the last three parameters.  We also check that
	// the library version is compatible with the one used at compile time,
	// in case we are using dynamically linked libraries.
	png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);
	if (png_ptr == NULL)
		throw runtime_error(NULL);

	// Allocate/initialize the image information data.
	png_info* info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_info**)NULL);
		throw runtime_error(NULL);
	}

	// Set error handling.  REQUIRED if you aren't supplying your own
	// error handling functions in the png_create_write_struct() call.
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw runtime_error(NULL);
	}

	// If you are using replacement write functions, instead of calling
	// png_init_io() here you would call
	png_set_write_fn(png_ptr, (void* )outputData, myPngWriteFunction, myPngFlushFunction);

	// compress with zlib level of 1 because everything else would be too slow (ratio from bmp to png is about 25% then)
	png_set_compression_level(png_ptr, Z_BEST_SPEED);

	png_set_IHDR(png_ptr, info_ptr, width, height, bitDepth, grayscale ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Write the file header information.
	png_write_info(png_ptr, info_ptr);

	// Write the image data.
	uint32_t scanlineLength = width*  /*(bitDepth / 8)*/ (grayscale ? 1 : 3);
	png_byte* data;
	if (bitDepth == 8) {
		uint8_t* iM = (uint8_t*)imageMatrix;
		for (uint32_t y = 0; y < height; y++) {
			data = iM + y * scanlineLength;
			png_write_rows(png_ptr, &data, 1);
		}
	}
	else if (bitDepth == 16) {
		png_set_swap(png_ptr);
		uint16_t* iM = (uint16_t*)imageMatrix;
		for (uint32_t y = 0; y < height; y++) {
			data = (png_byte*)(iM + y * scanlineLength);
			png_write_rows(png_ptr, &data, 1);
		}
	}
	else
		throw logic_error(NULL);

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void _debugPng(const wstring fileName, const wstring append, const void* data, const uint8_t bitDepth) {
	extern config* g_cfg;

	wstring newName(fileName);
	newName.erase(newName.find("."));
	newName.append(append);
	FILE* fOut;
	if ((fOut = fopen(newName.c_str(), "wb")) == 0)
		throw runtime_error(NULL);
	byteStorage out;

	createPng(g_cfg->imageResX, g_cfg->imageResY, bitDepth, true, data, &out);

	fwrite(out.getData(), 1, out.getLength(), fOut);
	if (fclose(fOut) != 0)
		throw runtime_error(NULL);
}

void debugPng(const wstring fileName, const wstring append, uint8_t* data) {
	_debugPng(fileName, append, data, 8);
}

void debugPng(const wstring fileName, const wstring append, uint16_t* data) {
	_debugPng(fileName, append, data, 16);
}
