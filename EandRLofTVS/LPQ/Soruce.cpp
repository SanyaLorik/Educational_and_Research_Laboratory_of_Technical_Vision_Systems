#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace std;
using namespace cv;
const double M_PI = 3.1415;

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
/*
    std::cout << "1" << std::endl;
    // Загрузка изображения
    cv::Mat image = cv::imread("C:/sobel.jpg", cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Ошибка загрузки изображения!" << std::endl;
        return -1;
    }

    std::cout << "2 " << std::endl;
    int window_rows = 32;
    int window_cols = 32;
    int step_rows = 16;
    int step_cols = 16;

    cv::Mat stft_result = stft(image, window_rows, window_cols, step_rows, step_cols);

    std::cout << "3 " << std::endl;
    // Вывод результата
    std::cout << "STFT вычислено. Размер: " << stft_result.size() << std::endl;

*/
    return 0;
}