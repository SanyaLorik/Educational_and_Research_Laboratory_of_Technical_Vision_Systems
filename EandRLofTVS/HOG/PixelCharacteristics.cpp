#include "PixelCharacteristics.h"

PixelCharacteristics::PixelCharacteristics(int magnitude, int orientation) : magnitude(magnitude)
{
	this->orientation = orientation < 0 ? orientation + 180 : orientation;
}