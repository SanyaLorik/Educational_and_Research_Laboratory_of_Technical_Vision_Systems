#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <cmath>
#include <chrono>

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

	unordered_map<uint8_t, HistogramValue> _histogram_map;
	int _histogramSize = 0;

	int _count_of_rows;
	int _count_of_cols;

	mutex _count_around_mutex;

public:

	Lbp(Mat* mat)
	{
		_original = *mat;
		_current = &_original;
	}

	Lbp(string fullPath)
	{
		_original = imread(fullPath, IMREAD_UNCHANGED);
		_current = &_original;
	}

	void crop()
	{
		int cellRows = _original.rows / CROP_RATIO;
		int cellCols = _original.cols / CROP_RATIO;

		_count_of_rows = cellRows * CROP_RATIO;
		_count_of_cols = cellCols * CROP_RATIO;

		Rect crop(0, 0, _count_of_cols, _count_of_rows);

		_original = _original(crop).clone();
		_current = &_original;
	}

	Mat& getCurrentMat()
	{
		return *_current;
	}

	void grayScaleUsingAvarageParallel()
	{
		_gray = _original.clone();
		_current = &_gray;

		int count_of_core = getCountOfCore();
		std::thread* threads = new std::thread[count_of_core];

		int count_of_rows = _count_of_rows / count_of_core;

		for (int i = 0; i < count_of_core; i++)
		{
			int initial_y = i * count_of_rows; // Правильный расчет начальной строки
			threads[i] = std::thread(&Lbp::grayScaleUsingAvarage, this, initial_y, count_of_rows);
		}

		for (int i = 0; i < count_of_core; i++)
			threads[i].join();

		delete[] threads;
	}

	Mat* grayScaleGradation()
	{
		_gray = _original.clone();
		_current = &_gray;

		int rows = _gray.rows;
		int cols = _gray.cols;

		for (int i = 0; i < rows; i++)
		{
			Vec3b* pxs = _gray.ptr<Vec3b>(i);
			for (int j = 0; j < cols; j++)
			{
				Vec3b color = pxs[j];
				int c = ((float)color[0]) * 0.114f
					+ ((float)color[1]) * 0.587f
					+ ((float)color[2]) * 0.299f;

				pxs[j] = Vec3b(c, c, c);
			}
		}

		return _current;
	}
	
	HistogramValue* countAroundParallel() 
	{
		int count_of_core = getCountOfCore();

		int count_of_rows = _count_of_rows / count_of_core;
		vector<thread> threads(count_of_core);

		for (int i = 0; i < count_of_core; i++) 
		{
			int initial_y = i * count_of_rows;
			threads[i] = thread(&Lbp::countAround, this, initial_y, count_of_rows);
		}

		for (auto& t : threads) 
			t.join();

		_histogramSize = _histogram_map.size();
		HistogramValue* values = new HistogramValue[_histogramSize];

		int count_of_histogram = _histogramSize / count_of_core;
		vector<thread> fillThreads(count_of_core);

		for (int i = 0; i < count_of_core; i++) 
		{
			int initial_index = i * count_of_histogram;
			fillThreads[i] = thread(&Lbp::fillHistogram, this, values, initial_index, count_of_histogram);
		}

		for (auto& t : fillThreads)
			t.join();

		return values;
	}

private:

	/* Тут мб оптимизровать - не каждую итерпцию обращаться к строке */
	uint8_t countLocalAround(int x, int y)
	{
		const int length = 8;
		tuple<int, int> offsets[length] =
		{
			{ -1, 1 }, { 0, 1 }, { 1, 1 },
			{ 1, 0 }, { 1, -1 }, { 0, -1 },
			{ -1, -1 }, { -1, 0 }
		};

		uint8_t result = 0;
		int centrePixel = _original.ptr<Vec3b>(y)[x][0];

		for (int i = 0; i < length; i++)
		{
			int x_offset, y_offset;
			tie(x_offset, y_offset) = offsets[i];

			if (y + y_offset < 0)
				continue;

			if (y + y_offset >= _count_of_rows)
				continue;

			if (x + x_offset < 0)
				continue;

			if (x + x_offset >= _count_of_cols)
				continue;

			int pxl = _original.ptr<Vec3b>(y + y_offset)[x + x_offset][0];
			int number = centrePixel <= pxl ? 1 : 0;

			result |= number;
			result <<= 1;
		}

		return result;
	}

	void countAround(int initial_y, int count_of_rows)
	{
		int length_y = initial_y + count_of_rows;
		for (int y = initial_y; y < length_y; y++)
		{
			for (int x = 0; x < _count_of_cols; x++)
			{
				int number = countLocalAround(x, y);

				_count_around_mutex.lock();

				if (_histogram_map.find(number) == _histogram_map.end())
					_histogram_map[number] = HistogramValue(number);
				else
					_histogram_map[number].Add();

				_count_around_mutex.unlock();
			}
		}
	}

	void grayScaleUsingAvarage(int initial_y, int count_of_rows)
	{
		int rows = _gray.rows;
		int cols = _gray.cols;
		
		int length_y = initial_y + count_of_rows;
		for (int y = initial_y; y < length_y; y++)
		{
			Vec3b* pxs = _gray.ptr<Vec3b>(y);
			for (int x = 0; x < cols; x++)
			{
				Vec3b color = pxs[x];
				int c = (color[0] + color[1] + color[2]) / 3;
				pxs[x] = Vec3b(c, c, c);
			}
		}
	}

	int getCountOfCore()
	{
		return thread::hardware_concurrency();
	}

	void fillHistogram(HistogramValue* values, int initial_index, int lenght_values)
	{
		int lenght = initial_index + lenght_values;
		for (const auto& pair : _histogram_map)
		{
			if (lenght > initial_index)
				return;

			values[initial_index] = pair.second;
			initial_index++;
		}
	}
};