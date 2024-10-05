#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <cmath>
#include <complex>
#include <stdio.h>

using namespace cv;
using namespace std;

class Lpq_Fast
{
    const double M_PI = 3.1415926535897;

public:
    Mat original;
    Mat* forward;
    Mat* revers;

    int rows;
    int cols;

    Lpq_Fast(string fullPath)
    {
        original = imread(fullPath, IMREAD_GRAYSCALE); 
    }

    ~Lpq_Fast()
    {
        delete forward;
        delete revers;
        for (int i = 0; i < rows; i++)
        {
            delete[] dft[i];
        }
        delete[] dft;
    }
};
