#pragma once

#include "PixelCharacteristics.h"

using namespace std;

class Cell
{

private:
	const static int const SIZE_IN_CELL = 9;

public:
	int histogram[SIZE_IN_CELL];
	int size = 0;
	int index = 0;

public:

	Cell() : size(0), index(0) {}

	Cell(int size);

	void Add(PixelCharacteristics pxlChrst);

	void AddToHistogram(PixelCharacteristics pxlChrst);
};