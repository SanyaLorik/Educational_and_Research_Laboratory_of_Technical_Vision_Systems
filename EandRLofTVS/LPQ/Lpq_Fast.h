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

    complex<double>** dft;
    int rows;
    int cols;

    Lpq_Fast(string fullPath)
    {
        original = imread(fullPath, IMREAD_GRAYSCALE); // Загрузка изображения в градациях серого
        if (original.empty())
        {
            cerr << "Error loading image!" << endl;
            return;
        }

        original.convertTo(original, CV_64FC1); // Преобразование в CV_64FC1

        rows = original.rows;
        cols = original.cols;

        forward = new Mat(rows, cols, CV_64FC2);
        revers = new Mat(rows, cols, CV_64FC1); // Изменение на CV_64FC1
        dft = new complex<double>*[rows]; // Инициализация dft
        for (int i = 0; i < rows; i++)
        {
            dft[i] = new complex<double>[cols];
        }
    }

    ~Lpq_Fast() // Деструктор для освобождения памяти
    {
        delete forward;
        delete revers;
        for (int i = 0; i < rows; i++)
        {
            delete[] dft[i];
        }
        delete[] dft;
    }

    void Forward(int initial_y, int count_of_rows)
    {
        int length_y = initial_y + count_of_rows;
        for (int k = initial_y; k < length_y; k++)
        {
            for (int l = 0; l < cols; l++)
            {
                complex<double> sum(0.0, 0.0);
                for (int p = 0; p < rows; p++)
                {
                    for (int q = 0; q < cols; q++)
                    {
                        double theta = -2.0 * M_PI * (((double)k * p / rows) + ((double)l * q / cols));
                        double pixelValue = original.at<double>(p, q); // Использование double

                        complex<double> complexPixel(pixelValue, 0.0); // Комплексное число

                        sum += complexPixel * exp(complex<double>(0, theta));
                    }
                }
                Vec2d a;
                a[0] = std::real(sum); // Сохранение действительной части
                a[1] = std::imag(sum); // Сохранение мнимой части
                forward->at<Vec2d>(k, l) = a; // Запись в forward
            }
            //cout << k << endl;
        }

        imshow("awd", *forward);
    }
};
