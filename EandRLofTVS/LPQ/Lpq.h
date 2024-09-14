#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <cmath>

using namespace cv;
using namespace std;

class Lpq 
{

private:
	const int CROP_RATIO = 2;

	Mat _original;
	Mat _gray;
	Mat* _current;

	int _count_of_rows;
	int _count_of_cols;

public:
	Lpq(Mat* mat);

	Lpq(string fullPath);

	void crop();

	Mat& getCurrentMat();

	void grayScaleUsingAvarageParallel();

private:

	void grayScaleUsingAvarage(int initial_y, int count_of_rows);

	int getCountOfCore();
};