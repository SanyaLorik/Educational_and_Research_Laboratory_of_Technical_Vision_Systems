#pragma once

#include "PixelCharacteristics.h"

using namespace std;

class Cell
{

private:
	const static int const SIZE_IN_CELL = 9;

public:
	int Histogram[SIZE_IN_CELL];
	int Size = 0;
	int Index = 0;

public:

	Cell() : Size(0), Index(0) {}

	Cell(int size);

	void add(PixelCharacteristics pxlChrst);

	void addToHistogram(PixelCharacteristics pxlChrst);
};