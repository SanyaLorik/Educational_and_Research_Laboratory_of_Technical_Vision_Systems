#include "Source.h"
#include "../LE/ComplexNS.h"

int main()
{
	const string name = "C:/sobel.jpg";

	Lbp lbp(name);
	lbp.crop();

	auto now = std::chrono::system_clock::now();
	auto start_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

	lbp.grayScaleUsingAvarageParallel();
	HistogramValue* histogram = lbp.countAroundParallel();

	now = std::chrono::system_clock::now();
	auto now_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	std::cout << "Reduce: " << now_milliseconds - start_milliseconds << std::endl;

	for (int i = 0; i < 256; i++)
	{
		std::cout << histogram[i].Count << " ";
	}

	imshow("сжатчик", lbp.getCurrentMat());
	waitKey(0);

	return 0;
}