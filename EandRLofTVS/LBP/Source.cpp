#include "Source.h"
#include "../LE/ComplexNS.h"

int main()
{
	const string name = "C:/sobel.jpg";
	Lbp lbp(name);
	ComplexNS a = ComplexNS(1, 2);
	ComplexNS B = ComplexNS(1, 2);
	ComplexNS C = a + B;
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