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

#include "ComplexNS.h"

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

    // Рекурсивное вычисление БПФ одномерного массива
    vector<ComplexNS> recursive_fft(const vector<ComplexNS>& input)
    {
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
};