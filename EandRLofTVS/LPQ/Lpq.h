#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <thread>
#include <cmath>
#include <stdio.h>
#include <vector>

#include "../LE/ComplexNS.h"

using namespace cv;
using namespace std;

class Lpq
{
    const double M_PI = 3.1415926535897;
    const int SEGMENT = 8;

public:
    Mat original;

    vector<vector<ComplexNS>> _forward;
    vector<vector<uchar>> _invers;

    int rows;
    int cols;

    Lpq(string fullPath)
    {
        original = imread(fullPath, IMREAD_GRAYSCALE);

        rows = original.rows;
        cols = original.cols;
    }

    ~Lpq()
    {

    }

    void calculalte_fft2d();

    void calculalte_inverse_fft2D();

    // перевод в комплексную форму изображение
    void transformToComplex();

    // Двумерное БПФ с помощью одномерного БПФ
    void fft2D();

    // Обратное двумерное БПФ
    void inverse_fft2D();

    // Обратное одномерное БПФ
    vector<ComplexNS> inverse_fft(const vector<ComplexNS>& input);

    void transformToReal(const vector<vector<ComplexNS>>& inverse_F);

    vector<int>* CalculateHistogram(int kernelSize = 4, int step = 1);

    vector<vector<ComplexNS>> get_window(int y, int x, int kernelSize);

    void fft2D(vector<vector<ComplexNS>>& window, int kernelSize);

    vector<int> get_local_histogram(const vector<vector<ComplexNS>>& fft2d);

    void sum_locals_histograms(vector<int>& histogram, const vector<vector<int>>& locals);

    // Рекурсивное вычисление БПФ одномерного массива
    vector<ComplexNS> recursive_fft(const vector<ComplexNS>& input);
};