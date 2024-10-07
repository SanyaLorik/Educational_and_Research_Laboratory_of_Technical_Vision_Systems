#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <cmath>

#include "HistogramValue.h"

using namespace cv;
using namespace std;

class Lbp
{

private:
	double const PI = 3.1415926535f;
	const int SIZE_IN_CELL = 9;
	const int CROP_RATIO = 2;

	Mat _original;
	Mat _gray;
	Mat* _current;

	int _rows;
	int _cols;

	unordered_map<uint8_t, HistogramValue> _histogram_map = unordered_map<uint8_t, HistogramValue>(256);
	int _histogramSize = 0;

	mutex _count_around_mutex;

public:

	Lbp(Mat* mat);

	Lbp(string fullPath);

	void crop();

	Mat& getCurrentMat();

	void grayScaleUsingAvarageParallel();

	Mat* grayScaleGradation();
	
	HistogramValue* countAroundParallel();

private:

	uint8_t countLocalAround(int x, int y);

	void countAround(int initial_y, int count_of_rows);

	void grayScaleUsingAvarage(int initial_y, int count_of_rows);

	int getCountOfCore();

	void fillHistogram(HistogramValue* values, int initial_index, int lenght_values);
};