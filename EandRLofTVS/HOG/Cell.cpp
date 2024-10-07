#include "Cell.h"
#include <opencv2/core.hpp>

Cell::Cell(int size) : Size(size* size)
{
	fill(Histogram, Histogram + SIZE_IN_CELL, 0);
}

void Cell::add(PixelCharacteristics pxlChrst)
{
	Index++;
	addToHistogram(pxlChrst);
}

void Cell::addToHistogram(PixelCharacteristics pxlChrst)
{
	//cout << "histogramIndex " << histogramIndex << " pxlChrst.orientation  " << pxlChrst.orientation  <<  "\n";

	if (pxlChrst.Orientation > 160)
	{
		float ratio = (float)(180 - pxlChrst.Orientation) / 20.0;
		Histogram[8] += ratio * pxlChrst.Magnitude;
		Histogram[0] += (1 - ratio) * pxlChrst.Magnitude;
	}
	else if (pxlChrst.Orientation > 9 && ((float)pxlChrst.Orientation / 10.0f) / 2 != 0)
	{
		int histogramIndex = pxlChrst.Orientation < 30 ?
			(pxlChrst.Orientation / 180.0) * 9 + 1 :
			(pxlChrst.Orientation / 180.0) * 9;

		Histogram[histogramIndex] += pxlChrst.Magnitude / 2.0;
		Histogram[histogramIndex - 1] += pxlChrst.Magnitude / 2.0;

	}
	else
	{
		int histogramIndex = (pxlChrst.Orientation / 180.0) * 9;
		Histogram[histogramIndex] += pxlChrst.Magnitude;
	}
}