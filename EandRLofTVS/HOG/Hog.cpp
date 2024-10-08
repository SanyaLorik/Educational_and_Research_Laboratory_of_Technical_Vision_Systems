#include "Hog.h"

Hog::Hog(Mat* mat)
{
	_original = *mat;
	_current = &_original;

	cropOriginal();
}

Hog::Hog(string fullPath)
{
	_original = imread(fullPath, IMREAD_UNCHANGED);
	_current = &_original;

	cropOriginal();
}

void Hog::cropOriginal(int rate, int blockSize)
{
	this->_rate = rate;
	this->_block_size = blockSize;

	_cell_rows = _original.rows / rate;
	_cell_cols = _original.cols / rate;

	int rows = _cell_rows * rate;
	int cols = _cell_cols * rate;

	Rect crop(0, 0, cols, rows);

	_cells = new Cell[_cell_rows * _cell_cols];
	fill(_cells, _cells + (_cell_rows * _cell_cols), Cell(rate));

	_block_rows = _cell_rows - (blockSize - 1);
	_block_cols = _cell_cols - (blockSize - 1);
	_blocks = new Block[_block_rows * _block_cols];
	fill(_blocks, _blocks + (_block_rows * _block_cols), Block(blockSize));

	_original = _original(crop).clone();
	_current = &_original;
}

Mat* Hog::getCurrentMat()
{
	return _current;
}

Mat* Hog::grayScaleUsingAvarage()
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
			int c = (color[0] + color[1] + color[2]) / 3;
			pxs[j] = Vec3b(c, c, c);
		}
	}

	return _current;
}

Mat* Hog::gauss(int kernelSize, double sigma)
{
	double** kernel = createGaussianKernel(kernelSize, sigma);
	_current = applyConvolution(kernel, kernelSize);

	return _current;
}

Mat* Hog::sobel()
{
	Mat* sobel = new Mat((*_current).clone());

	const int size = 3;

	double** mask_x = new double* [size]
		{
			new double[size] { -1, -2, -1},
				new double[size] { 0, 0, 0},
				new double[size] { 1, 2, 1},
		};

	double** mask_y = transpose(mask_x, size, size);

	int rows = sobel->rows;
	int cols = sobel->cols;

	for (int y = 0; y < rows; y++)
	{
		Vec3b* pxs_sobel = sobel->ptr<Vec3b>(y);
		for (int x = 0; x < cols; x++)
		{
			int r_x = 0, g_x = 0, b_x = 0;
			int r_y = 0, g_y = 0, b_y = 0;

			applySobelMask(x, y, mask_x, &r_x, &g_x, &b_x);
			applySobelMask(x, y, mask_y, &r_y, &g_y, &b_y);

			int r = sqrt(r_x * r_x + r_y * r_y);
			int g = sqrt(g_x * g_x + g_y * g_y);
			int b = sqrt(b_x * b_x + b_y * b_y);

			double orientation = r_x == 0 ? PI / 2 : atan(r_y / r_x);
			orientation *= 180.0 / PI;

			Vec3b mask(r, g, b);
			pxs_sobel[x] = mask;

			//int index = (y / (rate * rate)) * cols + (x / (rate * rate));
			int indexCell = (int)(y / _rate) * (int)(cols / _rate) + (int)(x / _rate);
			_cells[indexCell].add(PixelCharacteristics(r, orientation));
		}
	}

	for (int i = 0; i < size; ++i) {
		delete[] mask_x[i];
		delete[] mask_y[i];
	}

	delete[] mask_x;
	delete[] mask_y;

	_current = sobel;
	return _current;
}

void Hog::fillBlocks()
{
	int indexBlock = 0;

	for (int x = 0; x < _block_rows; x++)
	{
		for (int y = 0; y < _block_cols; y++)
		{
			int indexCell = indexBlock;
			for (int yCounter = 0; yCounter < _block_size; yCounter++)
			{
				for (int xCounter = 0; xCounter < _block_size; xCounter++)
				{
					_blocks[indexBlock].Add(_cells[indexCell].Histogram);
					indexCell++;
				}

				indexCell = indexBlock + _cell_cols;
			}

			indexBlock++;
		}
	}
}

float* Hog::getHistogram()
{
	/*
	* может быть очищать паммять
	* delete[] blockHistogram;
	*/
	int lengthHistogram = (_block_rows * _block_cols) * (_block_size * _block_size) * SIZE_IN_CELL;
	float* histogram = new float[lengthHistogram];
	int lengthBlock = _block_rows * _block_cols;

	int index = 0;

	for (int i = 0; i < lengthBlock; i++)
	{
		Block block = _blocks[i];
		int size = block.CalculateSize();
		float* blockHistogram = block.NormalizeHistogram();

		for (int j = 0; j < size; j++)
		{
			histogram[index] = blockHistogram[j]; // Копируем значения
			index++;
		}
	}

	_length_histogram = lengthHistogram;

	return histogram;
}

int Hog::getLengthHistogram()
{
	return _length_histogram;
}

Mat* Hog::applyConvolution(double** kernel, int kernelSize)
{
	int rows = (*_current).rows;
	int cols = (*_current).cols;

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

					Vec3b pxl = (*_current).ptr<Vec3b>(y + ky)[x + kx];
					float maskValue = kernel[ky][kx];

					sumR += pxl[0] * maskValue;
					sumG += pxl[1] * maskValue;
					sumB += pxl[2] * maskValue;
				}
			}

			(*_current).ptr<Vec3b>(y)[x] = Vec3b(sumR, sumG, sumB);
		}
	}

	return _current;
}

double** Hog::createGaussianKernel(int kernelSize, double sigma)
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
			double value = guassian(x, y, sigma);
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

double Hog::guassian(int x, int y, double sigma)
{
	return (1 / (2 * PI * pow(sigma, 2))) * exp(-((x * x + y * y) / (2 * pow(sigma, 2))));
}

double** Hog::transpose(double** matrix, int rows, int cols)
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

void Hog::applySobelMask(int x, int y, double** mask, int* r, int* g, int* b)
{
	int rows = _gray.rows;
	int cols = _gray.cols;

	for (int ym = 0, ky = -1; ky <= 1; ky++, ym++)
	{
		if (y + ky < 0)
			continue;

		if (y + ky >= rows)
			continue;

		Vec3b* pxls = _gray.ptr<Vec3b>(y + ky);

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