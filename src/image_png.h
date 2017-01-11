#pragma once
#include "image_helper.h"

void debugPng(const string fileName, const string append, uint8_t* data);
void debugPng(const string fileName, const string append, uint16_t* data);
void createPng(const uint32_t width, const uint32_t height, const uint8_t bitDepth, const bool grayscale, const void* imageMatrix, byteStorage* outputData);
