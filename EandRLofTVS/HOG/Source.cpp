#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cmath>
#include "Hog.h"

using namespace cv;
using namespace std;

int main()
{
	const string name = "C:/sobel.jpg";

	Hog hog(name);

	int kernelSize = 3;
	double sigma = 1;
	int rate = 8;

	hog.GrayScaleUsingAvarage();
	hog.Gauss(kernelSize, sigma);
	hog.Sobel();

	imshow("—жатчик", *hog.GetCurrentMat());
	int k = waitKey(0);

	return 0;
}