#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cmath>
#include "Hog.h"

using namespace cv;
using namespace std;

Vec3b* dif_mask(Vec3b prev, Vec3b next);

Vec3b* dif_mask_ratio(Vec3b prevVec, Vec3b nextVec, int prevRat, int nextRat);

void gauss();

void horiz_dif_mask();

Mat sobel(Mat img, Mat imgGray);

int main()
{
	const string name = "C:/sobel.jpg";

	Hog hog(name);

	int kernelSize = 9;
	double sigma = 4;
	int rate = 8;

	hog.GrayScaleUsingAvarage();
	//hog.Gauss(kernelSize, sigma);
	hog.Sobel();

	imshow("—жатчик", *hog.GetCurrentMat());
	int k = waitKey(0);

	return 0;
}

void gauss()
{
	const string name = "C:/sobel.jpg";
	Mat original = imread(name, IMREAD_UNCHANGED);

	int rows = original.rows;
	int cols = original.cols;

	const float sigma = 1.0f;

	for (int i = 0; i < rows; i++)
	{
		Vec3b* pxs = original.ptr<Vec3b>(i);
		for (int j = 0; j < cols; j++)
		{
			Vec3b* color = &pxs[j];
			color[0] = color[0] * (1 / sqrt(2 * 3.1415f * sigma * sigma) * exp(-(i * i + j * j) / (2 * sigma * sigma)));
			color[1] = color[1] * (1 / sqrt(2 * 3.1415f * sigma * sigma) * exp(-(i * i + j * j) / (2 * sigma * sigma)));
			color[2] = color[3] * (1 / sqrt(2 * 3.1415f * sigma * sigma) * exp(-(i * i + j * j) / (2 * sigma * sigma)));
			pxs[j] = *color;
		}
	}

	imshow("—жатчик", original);
	int k = waitKey(0); // Wait for a keystroke in the window
}

Mat sobel(Mat img, Mat imgGray)
{
	int rows = imgGray.rows;
	int cols = imgGray.cols;

	for (int i = 1; i < rows - 1; i++)
	{
		Vec3b* pxs_1 = imgGray.ptr<Vec3b>(i - 1);
		Vec3b* pxs_2 = imgGray.ptr<Vec3b>(i);
		Vec3b* pxs_3 = imgGray.ptr<Vec3b>(i + 1);

		Vec3b* pxs_orig = img.ptr<Vec3b>(i);

		Vec3b sobel_mask[9];

		for (int j = 1; j < cols - 1; j++)
		{
			int ratio = 1;

			sobel_mask[0] = pxs_1[j - 1] * ratio;
			sobel_mask[1] = pxs_1[j] * ratio;
			sobel_mask[2] = pxs_1[j + 1] * ratio;

			sobel_mask[3] = pxs_2[j - 1] * ratio;
			sobel_mask[4] = pxs_2[j] * ratio;
			sobel_mask[5] = pxs_2[j + 1] * ratio;

			sobel_mask[6] = pxs_3[j - 1] * ratio;
			sobel_mask[7] = pxs_3[j] * ratio;
			sobel_mask[8] = pxs_3[j + 1] * ratio;

			int b_x = (sobel_mask[6][0] + 2 * sobel_mask[7][0] + sobel_mask[8][0]) - (sobel_mask[0][0] + 2 * sobel_mask[1][0] + sobel_mask[2][0]);
			int g_x = (sobel_mask[6][1] + 2 * sobel_mask[7][1] + sobel_mask[8][1]) - (sobel_mask[0][1] + 2 * sobel_mask[1][1] + sobel_mask[2][1]);
			int r_x = (sobel_mask[6][2] + 2 * sobel_mask[7][2] + sobel_mask[8][2]) - (sobel_mask[0][2] + 2 * sobel_mask[1][2] + sobel_mask[2][2]);

			int b_y = (sobel_mask[2][0] + 2 * sobel_mask[5][0] + sobel_mask[8][0]) - (sobel_mask[0][0] + 2 * sobel_mask[3][0] + sobel_mask[6][0]);
			int g_y = (sobel_mask[2][1] + 2 * sobel_mask[5][1] + sobel_mask[8][1]) - (sobel_mask[0][1] + 2 * sobel_mask[3][1] + sobel_mask[6][1]);
			int r_y = (sobel_mask[2][2] + 2 * sobel_mask[5][2] + sobel_mask[8][2]) - (sobel_mask[0][2] + 2 * sobel_mask[3][2] + sobel_mask[6][2]);

			int b = sqrt(b_x * b_x + b_y * b_y);
			int g = sqrt(g_x * g_x + g_y * g_y);
			int r = sqrt(r_x * r_x + r_y * r_y);

			Vec3b* mask = new Vec3b(b, g, r);
			pxs_orig[j] = *mask;
		}
	}

	return img;
}

void horiz_dif_mask()
{
	const string name = "C:/photo_2024-07-10_15-00-10.jpg";
	Mat img = imread(name, IMREAD_UNCHANGED);

	int rows = img.rows;
	int cols = img.cols;

	for (int i = 0; i < rows; i++)
	{
		Vec3b* pxs = img.ptr<Vec3b>(i);
		Vec3b curOrig = pxs[1];

		for (int j = 1; j < cols - 1; j++)
		{
			Vec3b previous = curOrig;
			Vec3b next = pxs[j + 1];

			Vec3b* mask = dif_mask(previous, next);

			curOrig = pxs[j];

			//cout << mask << endl;

			pxs[j] = *mask;
		}
	}

	imshow("—жатчик", img);
	int k = waitKey(0); // Wait for a keystroke in the window
}

Vec3b* dif_mask(Vec3b prev, Vec3b next)
{
	int r = -prev[2] + next[2];
	int g = -prev[1] + next[1];
	int b = -prev[0] + next[0];

	Vec3b* color = new Vec3b(b, g, r);
	return color;
}

Vec3b* dif_mask_ratio(Vec3b prevVec, Vec3b nextVec, int prevRat, int nextRat)
{
	int r = (prevRat * prevVec[2]) + (nextRat * nextVec[2]);
	int g = (prevRat * prevVec[1]) + (nextRat * nextVec[1]);
	int b = (prevRat * prevVec[0]) + (nextRat * nextVec[0]);

	Vec3b* color = new Vec3b(b, g, r);
	return color;
}