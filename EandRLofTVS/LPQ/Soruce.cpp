#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <thread>
#include "Lpq.h"

using namespace std;
using namespace cv;

int main()
{
    const string name = "C:/sobel.jpg";
    Lpq lpq(name);
    lpq.calculalte_fft2d();
    lpq.calculalte_inverse_fft2D();

    cv::Mat fft = cv::Mat(lpq.original.rows, lpq.original.cols, cv::IMREAD_GRAYSCALE);
    for (int i = 0; i < lpq._invers.size(); i++) {
        uchar* pixles = fft.ptr<uchar>(i);
        for (int j = 0; j < lpq._invers[0].size(); j++) {
            //cout << inverse_image[i][j] << " ";
            pixles[j] = static_cast<uchar>(lpq._invers[i][j]);
        }
    }
    vector<int> histogram = *lpq.CalculateHistogram();
    for (size_t i = 0; i < histogram.size(); i++)
    {
        cout << histogram[i] << " ";
    }
    cout << endl;
    cv::imshow("awda", fft);
    waitKey(0);

    return 0;
}