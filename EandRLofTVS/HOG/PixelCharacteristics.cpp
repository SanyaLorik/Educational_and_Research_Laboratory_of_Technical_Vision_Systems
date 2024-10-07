#include "PixelCharacteristics.h"

PixelCharacteristics::PixelCharacteristics(int magnitude, int orientation) : Magnitude(magnitude)
{
	this->Orientation = orientation < 0 ? orientation + 180 : orientation;
}