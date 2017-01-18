#pragma once
#include "definitions.h"

struct fileInf {
	std::wstring name;
	uint64_t size;
};

list<fileInf> getFiles(wstring dir, wstring extension);
void deleteFiles(list<fileInf> files);
