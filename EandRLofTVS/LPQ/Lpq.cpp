#include "Lpq.h"

Lpq::Lpq(Mat* mat)
{
	_original = *mat;
	_current = &_original;
}

Lpq::Lpq(string fullPath)
{
	_original = imread(fullPath, IMREAD_UNCHANGED);
	_current = &_original;
}

void Lpq::crop()
{
	int cellRows = _original.rows / CROP_RATIO;
	int cellCols = _original.cols / CROP_RATIO;

	_count_of_rows = cellRows * CROP_RATIO;
	_count_of_cols = cellCols * CROP_RATIO;

	Rect crop(0, 0, _count_of_cols, _count_of_rows);

	_original = _original(crop).clone();
	_current = &_original;
}

Mat& Lpq::getCurrentMat()
{
	return *_current;
}

void Lpq::grayScaleUsingAvarageParallel()
{
	_gray = _original.clone();
	_current = &_gray;

	int count_of_core = getCountOfCore();
	thread* threads = new thread[count_of_core];

	int count_of_rows = _count_of_rows / count_of_core;

	for (int i = 0; i < count_of_core; i++)
	{
		int initial_y = i * count_of_rows; // ѕравильный расчет начальной строки
		threads[i] = thread(&Lpq::grayScaleUsingAvarage, this, initial_y, count_of_rows);
	}

	for (int i = 0; i < count_of_core; i++)
		threads[i].join();

	delete[] threads;
}

void Lpq::createHanningWindow()
{
	_hanning_window = new double* [_count_of_rows_hannah];

	for (size_t i = 0; i < _count_of_rows_hannah; i++)
		_hanning_window[i] = new double[_count_of_cols_hannah];

	int y_indent = _count_of_rows_hannah / 2;
	int x_indent = _count_of_cols_hannah / 2;

	for (int y = -y_indent; y <= y_indent; y++)
	{
		for (int x = -x_indent; x <= x_indent; x++)
		{
			double value = formulaOfHannah(x, y);
			_hanning_window[y + y_indent][x + x_indent] = value;
		}
	}
}

void Lpq::grayScaleUsingAvarage(int initial_y, int count_of_rows)
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

int Lpq::getCountOfCore()
{
	return thread::hardware_concurrency();
}

double Lpq::formulaOfHannah(int x, int y)
{
	return 
		0.5 * (1 - cos(2 * CV_PI * x / _count_of_cols_hannah)) * 
		0.5 * (1 - cos(2 * CV_PI * y / _count_of_rows_hannah));
}