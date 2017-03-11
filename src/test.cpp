#include "test.h"
#include <iostream>
#include <stdlib.h>
#include "definitions.h"
#include "image_helper.h"
#include "math_helpers.h"
#include "image_bmp.h"

int main (int argc, char *argv[])
{
	auto inputData = bmp_read("test.bmp", 480122, 400, 400);
	return 0;
}
