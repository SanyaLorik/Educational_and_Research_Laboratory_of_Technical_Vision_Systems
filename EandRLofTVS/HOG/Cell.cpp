#include "Cell.h"
#include <opencv2/core.hpp>

Cell::Cell(int size) : size(size* size)
{
	fill(histogram, histogram + SIZE_IN_CELL, 0);
}

void Cell::Add(PixelCharacteristics pxlChrst)
{
	index++;
	AddToHistogram(pxlChrst);
}

void Cell::AddToHistogram(PixelCharacteristics pxlChrst)
{
	//cout << "histogramIndex " << histogramIndex << " pxlChrst.orientation  " << pxlChrst.orientation  <<  "\n";

	if (pxlChrst.orientation > 160)
	{
		float ratio = (float)(180 - pxlChrst.orientation) / 20.0;
		histogram[8] += ratio * pxlChrst.magnitude;
		histogram[0] += (1 - ratio) * pxlChrst.magnitude;
	}
	else if (pxlChrst.orientation > 9 && ((float)pxlChrst.orientation / 10.0f) / 2 != 0)
	{
		int histogramIndex = pxlChrst.orientation < 30 ?
			(pxlChrst.orientation / 180.0) * 9 + 1 :
			(pxlChrst.orientation / 180.0) * 9;

		histogram[histogramIndex] += pxlChrst.magnitude / 2.0;
		histogram[histogramIndex - 1] += pxlChrst.magnitude / 2.0;

	}
	else
	{
		int histogramIndex = (pxlChrst.orientation / 180.0) * 9;
		histogram[histogramIndex] += pxlChrst.magnitude;
	}
}