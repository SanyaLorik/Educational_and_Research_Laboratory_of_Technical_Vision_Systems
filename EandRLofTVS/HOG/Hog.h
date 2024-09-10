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

/*
* Расписать попунктно алгоритм хз здесь
*/

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

	Hog(Mat* mat)
	{
		original = *mat;
		current = &original;

		CropOriginal();
	}

	Hog(string fullPath)
	{
		original = imread(fullPath, IMREAD_UNCHANGED);
		current = &original;

		CropOriginal();
	}

	void CropOriginal(int rate = 8, int blockSize = 2)
	{
		this->rate = rate;
		this->blockSize = blockSize;

		cellRows = original.rows / rate;
		cellCols = original.cols / rate;

		int rows = cellRows * rate;
		int cols = cellCols * rate;

		Rect crop(0, 0, cols, rows);

		cells = new Cell[cellRows * cellCols];
		fill(cells, cells + (cellRows * cellCols), Cell(rate));

		blockRows = cellRows - (blockSize - 1);
		blockCols = cellCols - (blockSize - 1);
		blocks = new Block[blockRows * blockCols];
		fill(blocks, blocks + (blockRows * blockCols), Block(blockSize));

		original = original(crop).clone();
		current = &original;
	}

	Mat* GetCurrentMat()
	{
		return current;
	}

	Mat* GrayScaleUsingAvarage()
	{
		gray = original.clone();
		current = &gray;

		int rows = gray.rows;
		int cols = gray.cols;

		for (int i = 0; i < rows; i++)
		{
			Vec3b* pxs = gray.ptr<Vec3b>(i);
			for (int j = 0; j < cols; j++)
			{
				Vec3b color = pxs[j];
				int c = (color[0] + color[1] + color[2]) / 3;
				pxs[j] = Vec3b(c, c, c);
			}
		}

		return current;
	}

	Mat* Gauss(int kernelSize, double sigma)
	{
		double** kernel = CreateGaussianKernel(kernelSize, sigma);
		current = ApplyConvolution(kernel, kernelSize);

		return current;
	}

	Mat* Sobel()
	{
		Mat* sobel = new Mat((*current).clone());

		const int size = 3;

		double** mask_x = new double* [size]
			{
				new double[size] { -1, -2, -1},
					new double[size] { 0, 0, 0},
					new double[size] { 1, 2, 1},
			};

		double** mask_y = Transpose(mask_x, size, size);

		int rows = sobel->rows;
		int cols = sobel->cols;

		for (int y = 0; y < rows; y++)
		{
			Vec3b* pxs_sobel = sobel->ptr<Vec3b>(y);
			for (int x = 0; x < cols; x++)
			{
				int r_x = 0, g_x = 0, b_x = 0;
				int r_y = 0, g_y = 0, b_y = 0;

				ApplySobelMask(x, y, mask_x, &r_x, &g_x, &b_x);
				ApplySobelMask(x, y, mask_y, &r_y, &g_y, &b_y);

				int r = sqrt(r_x * r_x + r_y * r_y);
				int g = sqrt(g_x * g_x + g_y * g_y);
				int b = sqrt(b_x * b_x + b_y * b_y);

				double orientation = r_x == 0 ? PI / 2 : atan(r_y / r_x);
				orientation *= 180.0 / PI;

				Vec3b mask(r, g, b);
				pxs_sobel[x] = mask;

				//int index = (y / (rate * rate)) * cols + (x / (rate * rate));
				int indexCell = (int)(y / rate) * (int)(cols / rate) + (int)(x / rate);
				cells[indexCell].Add(PixelCharacteristics(r, orientation));
			}
		}

		for (int i = 0; i < size; ++i) {
			delete[] mask_x[i];
			delete[] mask_y[i];
		}

		delete[] mask_x;
		delete[] mask_y;

		current = sobel;
		return current;
	}

	void FillBlocks()
	{
		int indexBlock = 0;

		for (int x = 0; x < blockRows; x++)
		{
			for (int y = 0; y < blockCols; y++)
			{
				int indexCell = indexBlock;
				for (int yCounter = 0; yCounter < blockSize; yCounter++)
				{
					for (int xCounter = 0; xCounter < blockSize; xCounter++)
					{
						blocks[indexBlock].Add(cells[indexCell].histogram);
						indexCell++;
					}

					indexCell = indexBlock + cellCols;
				}

				indexBlock++;
			}
		}
	}

	float* GetHistogram()
	{
		/*
		* может быть очищать паммять
		* delete[] blockHistogram;
		*/
		int lengtHistogram = (blockRows * blockCols) * (blockSize * blockSize) * SIZE_IN_CELL;
		float* histogram = new float[lengtHistogram];
		int lengthBlock = blockRows * blockCols;

		int index = 0;

		for (int i = 0; i < lengthBlock; i++)
		{
			Block block = blocks[i];
			int size = block.CalculateSize();
			float* blockHistogram = block.NormalizeHistogram();

			for (int j = 0; j < size; j++)
			{
				histogram[index] = blockHistogram[j]; // Копируем значения
				index++;
			}
		}

		return histogram;
	}

public:

	Mat* ApplyConvolution(double** kernel, int kernelSize)
	{
		int rows = (*current).rows;
		int cols = (*current).cols;

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				int sumR = 0;
				int sumG = 0;
				int sumB = 0;

				for (int ky = 0; ky < kernelSize; ky++)
				{
					for (int kx = 0; kx < kernelSize; kx++)
					{
						if (y + ky < 0)
							continue;

						if (y + ky >= rows)
							continue;

						if (x + kx < 0)
							continue;

						if (x + kx >= cols)
							continue;

						Vec3b pxl = (*current).ptr<Vec3b>(y + ky)[x + kx];
						float maskValue = kernel[ky][kx];

						sumR += pxl[0] * maskValue;
						sumG += pxl[1] * maskValue;
						sumB += pxl[2] * maskValue;
					}
				}

				(*current).ptr<Vec3b>(y)[x] = Vec3b(sumR, sumG, sumB);
			}
		}

		return current;
	}

	double** CreateGaussianKernel(int kernelSize, double sigma)
	{
		double** kernel = new double* [kernelSize];

		for (size_t i = 0; i < kernelSize; i++)
			kernel[i] = new double[kernelSize];

		int indent = kernelSize / 2;
		double sum = 0.0f;

		for (int y = -indent; y <= indent; y++)
		{
			for (int x = -indent; x <= indent; x++)
			{
				double value = Guassian(x, y, sigma);
				sum += value;
				kernel[y + indent][x + indent] = value;
			}
		}

		for (size_t i = 0; i < kernelSize; i++)
		{
			for (size_t j = 0; j < kernelSize; j++)
				kernel[i][j] /= sum;
		}

		return kernel;
	}

	double Guassian(int x, int y, double sigma)
	{
		return (1 / (2 * PI * pow(sigma, 2))) * exp(-((x * x + y * y) / (2 * pow(sigma, 2))));
	}

	double** Transpose(double** matrix, int rows, int cols)
	{
		double** res = new double* [cols];

		for (size_t i = 0; i < cols; i++)
		{
			res[i] = new double[rows];
			for (size_t j = 0; j < rows; j++)
				res[i][j] = matrix[j][i];
		}

		return res;
	}

	void ApplySobelMask(int x, int y, double** mask, int* r, int* g, int* b)
	{
		int rows = gray.rows;
		int cols = gray.cols;

		for (int ym = 0, ky = -1; ky <= 1; ky++, ym++)
		{
			if (y + ky < 0)
				continue;

			if (y + ky >= rows)
				continue;

			Vec3b* pxls = gray.ptr<Vec3b>(y + ky);

			for (int xm = 0, kx = -1; kx <= 1; kx++, xm++)
			{
				if (x + kx < 0)
					continue;

				if (x + kx >= cols)
					continue;

				int ratio = mask[ym][xm];
				if (ratio == 0)
					continue;

				Vec3b pxl = pxls[x + kx];

				*r += pxl[0] * ratio;
				*g += pxl[1] * ratio;
				*b += pxl[2] * ratio;
			}
		}
	}
};