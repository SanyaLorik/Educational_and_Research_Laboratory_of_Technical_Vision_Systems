#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <cmath>
#include "Lbp.h"

using namespace cv;
using namespace std;

int main()
{
	const string name = "C:/sobel.jpg";
	Lbp lbp(name);

	lbp.Crop();
	lbp.GrayScaleUsingAvarage();
	lbp.CountAround();

	imshow("сжатчик", lbp.GetCurrentMat());
	waitKey(0);

	return 0;
}