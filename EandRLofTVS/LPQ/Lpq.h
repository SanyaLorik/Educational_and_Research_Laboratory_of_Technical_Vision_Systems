#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <cmath>
#include <complex>

using namespace cv;
using namespace std;

class Lpq 
{

private:
	const int CROP_RATIO = 2;

	Mat _original;
	Mat _gray;
	Mat* _current;

	int _rows;
	int _cols;

	// 2D DFT
	complex<float>** _dft;
	const int _size_window = 3;
	int _rows_window;
	int _cols_window;

	// Hannah's window
	const int _rows_hannah = 3;
	const int _cols_hannah = 3;
	double** _hanning_window;

public:
	Lpq(Mat* mat);

	Lpq(string fullPath);

	void crop();

	Mat& getCurrentMat();

	void grayScaleUsingAvarageParallel();

	void createHanningWindow();

	void calculateDft();

private:

	void grayScaleUsingAvarage(int initial_y, int count_of_rows);

	int getCountOfCore();
	
	double formulaOfHannah(int x, int y);
};