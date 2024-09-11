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

/*
* Расписать попунктно алгоритм хз здесь
*/

class Lbp
{

private:
	double const PI = 3.1415926535f;
	const int SIZE_IN_CELL = 9;
	const int CROP_RATIO = 2;

	Mat original;
	Mat gray;
	Mat* current;

	int rows;
	int cols;

	unordered_map<uint8_t, HistogramValue> histogram;
	int histogramSize = 0;

public:

	Lbp(Mat* mat)
	{
		original = *mat;
		current = &original;
	}

	Lbp(string fullPath)
	{
		original = imread(fullPath, IMREAD_UNCHANGED);
		current = &original;
	}

	void Crop()
	{
		int cellRows = original.rows / CROP_RATIO;
		int cellCols = original.cols / CROP_RATIO;

		rows = cellRows * CROP_RATIO;
		cols = cellCols * CROP_RATIO;

		Rect crop(0, 0, cols, rows);

		original = original(crop).clone();
		current = &original;
	}

	Mat& GetCurrentMat()
	{
		return *current;
	}

	void GrayScaleUsingAvarageParallel()
	{
		gray = original.clone();
		current = &gray;

		int count_of_core = GetCountOfCore();
		std::thread* threads = new std::thread[count_of_core];

		int count_of_rows = rows / count_of_core;

		for (int i = 0; i < count_of_core; i++)
		{
			int initial_y = i * count_of_rows; // Правильный расчет начальной строки
			threads[i] = std::thread(&Lbp::GrayScaleUsingAvarage, this, initial_y, count_of_rows);
		}

		for (int i = 0; i < count_of_core; i++)
			threads[i].join();

		delete[] threads;
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

private:

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

	void GrayScaleUsingAvarage(int initial_y, int count_of_rows)
	{
		int rows = gray.rows;
		int cols = gray.cols;
		
		int length = initial_y + count_of_rows;
		for (int i = initial_y; i < length; i++)
		{
			Vec3b* pxs = gray.ptr<Vec3b>(i);
			for (int j = 0; j < cols; j++)
			{
				Vec3b color = pxs[j];
				int c = (color[0] + color[1] + color[2]) / 3;
				pxs[j] = Vec3b(c, c, c);
			}
		}
	}

	int GetCountOfCore()
	{
		return thread::hardware_concurrency();
	}
};