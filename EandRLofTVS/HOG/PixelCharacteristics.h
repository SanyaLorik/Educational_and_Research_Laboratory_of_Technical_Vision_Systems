#pragma once

class PixelCharacteristics
{
public:
	int magnitude;
	int orientation;

	PixelCharacteristics() = default;

	PixelCharacteristics(int magnitude, int orientation);
};

