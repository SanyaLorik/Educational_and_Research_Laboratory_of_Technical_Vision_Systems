#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cmath>
#include <math.h>

#include "HistogramValue.h"
#include "PixelCharacteristics.h"
#include "Cell.h"
#include "Block.h"

using namespace cv;
using namespace std;

/*
* Расписать попунктно алгоритм хз здесь
*/

class Lbp
{

private:
	double const PI = 3.1415926535f;
	const int SIZE_IN_CELL = 9;

	Mat original;
	Mat gray;
	Mat* current;
	Cell* cells;
	Block* blocks;

	int rows;
	int cols;

	int cellRows;
	int cellCols;
	int rate = 8;

	int blockRows;
	int blockCols;
	int blockSize = 2;

	unordered_map<uint8_t, HistogramValue> histogram;
	int histogramSize = 0;

public:

	Lbp(Mat* mat)
	{
		original = *mat;
		current = &original;

		CropOriginal();
	}

	Lbp(string fullPath)
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

		rows = cellRows * rate;
		cols = cellCols * rate;

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

	Mat* GrayScaleGradation()
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
				int c = ((float)color[0]) * 0.114f
					+ ((float)color[1]) * 0.587f
					+ ((float)color[2]) * 0.299f;

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

				//ApplySobelMask(x, y, mask_x, &r_x, &g_x, &b_x);
				//ApplySobelMask(x, y, mask_y, &r_y, &g_y, &b_y);

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

	/* Мб сохранять внутри уже сделанные значения*/
	HistogramValue* CountAround()
	{
		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				int number = CountLocalAround(x, y);
				if (histogram.find(number) == histogram.end())
					histogram[number] = HistogramValue(number);
				else
					histogram[number].Add();
			}
		}

		histogramSize = histogram.size();
		HistogramValue* values = new HistogramValue[histogramSize];

		int index = 0;
		for (const auto& pair : histogram)
		{
			values[index] = pair.second;
			index++;
		}

		return values;
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

	/* Тут мб оптимизровать - не каждую итерпцию обращаться к строке */
	uint8_t CountLocalAround(int x, int y)
	{
		const int length = 8;
		tuple<int, int> offsets[length] =
		{
			{ -1, 1 }, { 0, 1 }, { 1, 1 },
			{ 1, 0 }, { 1, -1 }, { 0, -1 },
			{ -1, -1 }, { -1, 0 }
		};

		uint8_t result = 0;
		int centrePixel = original.ptr<Vec3b>(y)[x][0];

		for (int i = 0; i < length; i++)
		{
			int x_offset, y_offset;
			tie(x_offset, y_offset) = offsets[i];

			if (y + y_offset < 0)
				continue;

			if (y + y_offset >= rows)
				continue;

			if (x + x_offset < 0)
				continue;

			if (x + x_offset >= cols)
				continue;

			int pxl = original.ptr<Vec3b>(y + y_offset)[x + x_offset][0];
			int number = centrePixel <= pxl ? 1 : 0;

			result |= number;
			result <<= 1;
		}

		return result;
	}
};