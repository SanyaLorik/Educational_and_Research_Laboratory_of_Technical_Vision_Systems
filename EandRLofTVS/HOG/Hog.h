#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cmath>
#include <math.h>

#include "PixelCharacteristics.h"
#include "Cell.h"
#include "Block.h"

using namespace cv;
using namespace std;

class Hog
{

private:
	double const PI = 3.1415926535f;
	const int SIZE_IN_CELL = 9;

	Mat _original;
	Mat _gray;
	Mat* _current;
	Cell* _cells;
	Block* _blocks;

	int _cell_rows;
	int _cell_cols;
	int _rate = 8;

	int _block_rows;
	int _block_cols;
	int _block_size = 2;

public:

	Hog(Mat* mat);

	Hog(string fullPath);

	void cropOriginal(int rate = 8, int blockSize = 2);

	Mat* getCurrentMat();

	Mat* grayScaleUsingAvarage();

	Mat* gauss(int kernelSize, double sigma);

	Mat* sobel();

	void fillBlocks();

	float* getHistogram();

private:

	Mat* applyConvolution(double** kernel, int kernelSize);

	double** createGaussianKernel(int kernelSize, double sigma);

	double guassian(int x, int y, double sigma);

	double** transpose(double** matrix, int rows, int cols);

	void applySobelMask(int x, int y, double** mask, int* r, int* g, int* b);
};