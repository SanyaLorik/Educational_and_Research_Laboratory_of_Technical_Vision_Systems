#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <thread>
#include "Lpq_Fast.h"

using namespace std;
using namespace cv;

const double M_PI = 3.1415;


int getCountOfCore()
{
    return thread::hardware_concurrency();
}

std::vector<std::vector<double>> hanning_window(int rows, int cols) {
    std::vector<std::vector<double>> window(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            window[i][j] = 0.5 * (1 - cos(2 * M_PI * i / (rows - 1))) *
                0.5 * (1 - cos(2 * M_PI * j / (cols - 1)));
        }
    }
    return window;
}

std::vector<std::vector<std::complex<double>>> dft2d(const std::vector<std::vector<double>>& image) {
    int rows = image.size();
    int cols = image[0].size();
    std::vector<std::vector<std::complex<double>>> spectrum(rows, std::vector<std::complex<double>>(cols));

    for (int u = 0; u < rows; ++u) {
        for (int v = 0; v < cols; ++v) {

            std::complex<double> sum(0.0, 0.0);
            for (int x = 0; x < rows; ++x) {
                for (int y = 0; y < cols; ++y) {
                    double theta = -2 * M_PI * ((double)u * x / rows + (double)v * y / cols);
                    sum += image[x][y] * std::complex<double>(cos(theta), sin(theta));
                }
            }

            spectrum[u][v] = sum;
        }
    }
    return spectrum;
}

cv::Mat stft(const cv::Mat& image, int window_rows, int window_cols, int step_rows, int step_cols) {
    int num_windows_rows = (image.rows - window_rows) / step_rows + 1;
    int num_windows_cols = (image.cols - window_cols) / step_cols + 1;
    cv::Mat result(num_windows_rows, num_windows_cols, CV_64FC2); // Для комплексных чисел

    auto window = hanning_window(window_rows, window_cols);

    std::cout << "hanning_window" << std::endl;
    for (int m = 0; m < num_windows_rows; ++m) {
        for (int n = 0; n < num_windows_cols; ++n) {

            // применения окна Ханна
            std::vector<std::vector<double>> segment(window_rows, std::vector<double>(window_cols));
            for (int i = 0; i < window_rows; ++i) {
                for (int j = 0; j < window_cols; ++j) {
                    segment[i][j] = image.at<uchar>(m * step_rows + i, n * step_cols + j) * window[i][j];
                }
            }

            auto spectrum = dft2d(segment);
            for (int u = 0; u < window_rows; ++u) {
                for (int v = 0; v < window_cols; ++v) {
                    result.at<std::complex<double>>(m, n) = spectrum[u][v];
                }
            }
        }
    }
    return result;
}

int main() {
    
    const string name = "C:/sobel_2.jpg";
    Lpq_Fast lpq(name);
    lpq.calculalte_fft2d();
    lpq.calculalte_inverse_fft2D();


    cv::Mat fft = cv::Mat(lpq.original.rows, lpq.original.cols, cv::IMREAD_GRAYSCALE);
    for (int i = 0; i < lpq.invers.size(); i++) {
        uchar* pixles = fft.ptr<uchar>(i);
        for (int j = 0; j < lpq.invers[0].size(); j++) {
            //cout << inverse_image[i][j] << " ";
            pixles[j] = static_cast<uchar>(lpq.invers[i][j]);
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