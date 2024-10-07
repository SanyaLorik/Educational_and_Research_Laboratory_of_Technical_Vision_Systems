#pragma once

class PixelCharacteristics
{
public:
	int Magnitude;
	int Orientation;

	PixelCharacteristics() = default;

	PixelCharacteristics(int magnitude, int orientation);
};

