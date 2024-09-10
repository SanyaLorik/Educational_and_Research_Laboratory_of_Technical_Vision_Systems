#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;

#pragma once
class PixelCharacteristics
{
public:
	int magnitude;
	int orientation;

	PixelCharacteristics() = default;

	PixelCharacteristics(int magnitude, int orientation) : magnitude(magnitude)
	{
		this->orientation = orientation < 0 ? orientation + 180 : orientation;
	}
};

