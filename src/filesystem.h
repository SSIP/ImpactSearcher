#pragma once
#include "definitions.h"

struct fileInf {
	tstring name;
	uint64_t size;
};

list<fileInf> getFiles(tstring dir, tstring extension);
void deleteFiles(list<fileInf> files);
