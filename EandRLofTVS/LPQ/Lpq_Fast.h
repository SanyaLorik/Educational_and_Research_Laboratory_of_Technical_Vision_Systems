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

class Lpq_Fast
{
    const double M_PI = 3.1415926535897;

public:
    Mat original;

    vector<vector<ComplexNS>> forward;
    vector<vector<uchar>> invers;

    int rows;
    int cols;

    const int segment = 9;

    Lpq_Fast(string fullPath)
    {
        original = imread(fullPath, IMREAD_GRAYSCALE); 

        rows = original.rows;
        cols = original.cols;
    }

    ~Lpq_Fast()
    {

    }

    void calculalte_fft2d()
    {
        transformToComplex();
        fft2D();
    }

    void calculalte_inverse_fft2D() 
    {
        inverse_fft2D();
    }

    // перевод в комплексную форму изображение
    void transformToComplex()
    {
        forward = vector<vector<ComplexNS>>(rows, vector<ComplexNS>(cols));
        for (int y = 0; y < rows; y++) {
            uchar* pixels = original.ptr<uchar>(y);
            for (int x = 0; x < cols; x++) 
                forward[y][x] = ComplexNS(pixels[x], 0.0);
        }
    }

    // Двумерное БПФ с помощью одномерного БПФ
    void fft2D() 
    {
        // БПФ по строкам
        for (int y = 0; y < rows; y++) 
            forward[y] = recursive_fft(forward[y]);

        // БПФ по столбцам
        for (int x = 0; x < cols; x++) {

            vector<ComplexNS> column(rows);
            for (int y = 0; y < rows; y++)
                column[y] = forward[y][x];

            column = recursive_fft(column);
            for (int y = 0; y < rows; y++)
                forward[y][x] = column[y];
        }
    }

    // Обратное двумерное БПФ
    void inverse_fft2D() {

        // Обратное БПФ по столбцам
        vector<vector<ComplexNS>> inverse_F(rows, vector<ComplexNS>(cols));
        for (int j = 0; j < cols; j++) {
            vector<ComplexNS> column(rows);
            for (int i = 0; i < rows; i++)
                column[i] = forward[i][j];

            column = inverse_fft(column);
            for (int i = 0; i < rows; i++) 
                inverse_F[i][j] = column[i];
        }

        // Обратное БПФ по строкам
        for (int i = 0; i < rows; i++)
            inverse_F[i] = inverse_fft(inverse_F[i]);

        // Преобразование в вещественный формат
        transformToReal(inverse_F);
    }

    // Обратное одномерное БПФ
    vector<ComplexNS> inverse_fft(const vector<ComplexNS>& input) {
        int n = input.size();

        vector<ComplexNS> output = recursive_fft(input);
        for (int i = 0; i < n; i++)
            output[i] = output[i] * (1.0 / n);

        return output;
    }

    void transformToReal(const vector<vector<ComplexNS>>& inverse_F)
    {
        invers = vector<vector<uchar>>(rows, vector<uchar>(cols));
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++)
                invers[i][j] = inverse_F[i][j].real;
        }
    }

    vector<int>* CalculateHistogram(int kernelSize = 3, int step = 1)
    {
        vector<int>* histogram = new vector<int>(segment, 0);

        int offset = kernelSize / 2;
        int rowSize = rows - offset - offset;
        int colsSize = cols - offset - offset;
        vector<vector<int>> locals = vector<vector<int>>(rowSize, vector<int>(cols));

        for (int y = offset; y < rows - offset; y += step)
        {
            for (int x = offset; x < cols - offset; x += step)
            {
                vector<vector<ComplexNS>> area = get_window(y, x, kernelSize);
                fft2D(area, kernelSize);
                vector<int> local = get_local_histogram(area);

                locals[x - offset, y - offset] = local;
            }
        }

        sum_locals_histograms(*histogram, locals);

        return histogram;
    }

    vector<vector<ComplexNS>> get_window(int y, int x, int kernelSize) 
    {
        vector<vector<ComplexNS>> window = vector<vector<ComplexNS>>(kernelSize, vector<ComplexNS>(kernelSize));

        int half = kernelSize / 2;
        for (int yk = -half; yk <= half; yk++)
        {
            uchar* pixels = original.ptr<uchar>(y + yk);
            for (int xk = -half; xk <= half; xk++)
                window[yk + half][xk + half] = ComplexNS(pixels[x + xk], 0.0);
        }

        return window;
    }

    void fft2D(vector<vector<ComplexNS>>& window, int kernelSize)
    {
        // БПФ по строкам
        for (int i = 0; i < kernelSize; i++)
            window[i] = recursive_fft(window[i]);

        // БПФ по столбцам
        for (int j = 0; j < kernelSize; j++) {
            vector<ComplexNS> column(kernelSize);
            for (int i = 0; i < kernelSize; i++)
                column[i] = window[i][j];

            column = recursive_fft(column);
            for (int i = 0; i < kernelSize; i++)
                window[i][j] = column[i];
        }
    }

    vector<int> get_local_histogram(const vector<vector<ComplexNS>>& fft2d)
    {
        vector<int> histogram(segment, 0);
        const float ratio = 2.0f * M_PI / segment;

        for (int y = 0; y < fft2d.size(); y++)
        {
            for (int x = 0; x < fft2d[y].size(); x++)
            {
                float phase_value = fft2d[y][x].phase();
                int index = static_cast<int>(fmod(phase_value / ratio + segment, segment));
                histogram[index]++;
            }
        }

        return histogram;
    }


    void sum_locals_histograms(vector<int>& histogram, const vector<vector<int>>& locals)
    {
        for (int y = 0; y < locals.size(); y++)
        {
            for (int x = 0; x < locals[y].size(); x++)
            {
                for (int i = 0; i < histogram.size(); i++)
                    histogram[i] += locals[y][i];
            }
        }
    }

    // Рекурсивное вычисление БПФ одномерного массива
    vector<ComplexNS> recursive_fft(const vector<ComplexNS>& input) {
        int n = input.size();
        if (n == 1)
            return { input[0] };

        int half = n / 2;

        vector<ComplexNS> even(half);
        vector<ComplexNS> odd(half);

        for (int i = 0; i < half; i++) {
            even[i] = input[2 * i];
            odd[i] = input[2 * i + 1];
        }

        vector<ComplexNS> even_fft = recursive_fft(even);
        vector<ComplexNS> odd_fft = recursive_fft(odd);

        vector<ComplexNS> output(n);
        for (int k = 0; k < half; k++) {
            double angle = -2 * M_PI * k / n;
            ComplexNS factor = exp_complex(angle);
            output[k] = even_fft[k] + factor * odd_fft[k];
            output[k + half] = even_fft[k] - factor * odd_fft[k];
        }

        return output;
    }
};