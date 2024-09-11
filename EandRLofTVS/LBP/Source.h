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

	lbp.crop();

	auto now = std::chrono::system_clock::now();
	auto start_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	lbp.grayScaleUsingAvarageParallel();
	lbp.countAroundParallel();

	now = std::chrono::system_clock::now();
	auto now_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	std::cout << "Reduce: " << now_milliseconds - start_milliseconds << std::endl;

	imshow("сжатчик", lbp.getCurrentMat());
	waitKey(0);

	return 0;
}