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
		threads[i] = thread(&Lbp::grayScaleUsingAvarage, this, initial_y, count_of_rows);
	}

	for (int i = 0; i < count_of_core; i++)
		threads[i].join();

	delete[] threads;
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