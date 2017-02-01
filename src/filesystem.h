#pragma once
#include "definitions.h"

struct fileInf {
	string name;
	uint64_t size;
};

list<fileInf> getFiles(string dir, string extension);
void deleteFiles(list<fileInf> files);
