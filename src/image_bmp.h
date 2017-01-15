#pragma once
#include "image_helper.h"

uint8_t* bmp_read(const wstring fileName, const uint64_t fileSize, const uint32_t width, const uint32_t height);
