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

	Mat original;
	Mat gray;
	Mat* current;
	Cell* cells;
	Block* blocks;

	int cellRows;
	int cellCols;
	int rate = 8;

	int blockRows;
	int blockCols;
	int blockSize = 2;

public:

	Hog(Mat* mat);

	Hog(string fullPath);

	void CropOriginal(int rate = 8, int blockSize = 2);

	Mat* GetCurrentMat();

	Mat* GrayScaleUsingAvarage();

	Mat* Gauss(int kernelSize, double sigma);

	Mat* Sobel();

	void FillBlocks();

	float* GetHistogram();

private:

	Mat* ApplyConvolution(double** kernel, int kernelSize);

	double** CreateGaussianKernel(int kernelSize, double sigma);

	double Guassian(int x, int y, double sigma);

	double** Transpose(double** matrix, int rows, int cols);

	void ApplySobelMask(int x, int y, double** mask, int* r, int* g, int* b);
};