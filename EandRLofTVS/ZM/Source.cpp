#include <iostream>
#include <cmath>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

const double M_PI = 3.1415926535897;

// Структура для представления комплексного числа
struct ComplexNS {
    double real;
    double imag;

    ComplexNS(double r = 0, double i = 0) : real(r), imag(i) {}

    ComplexNS operator+(const ComplexNS& c) const {
        return ComplexNS(real + c.real, imag + c.imag);
    }

    ComplexNS operator-(const ComplexNS& c) const {
        return ComplexNS(real - c.real, imag - c.imag);
    }

    ComplexNS operator*(const ComplexNS& c) const {
        return ComplexNS(real * c.real - imag * c.imag, real * c.imag + imag * c.real);
    }

    ComplexNS operator*(const double& c) const {
        return ComplexNS(real * c, imag * c);
    }

    ComplexNS conjugate() const {
        return ComplexNS(real, -imag);
    }
};

// Вывод комплексного числа
ostream& operator<<(ostream& out, const ComplexNS& c) {
    out << c.real << " + " << c.imag << "i";
    return out;
}

// Вычисление экспоненты комплексного числа
ComplexNS exp_complex(double angle) {
    return ComplexNS(cos(angle), sin(angle));
}

// Рекурсивное вычисление БПФ одномерного массива
vector<ComplexNS> recursive_fft(const vector<ComplexNS>& input) {
    int n = input.size();
    if (n == 1) {
        return { input[0] };
    }

    vector<ComplexNS> even(n / 2);
    vector<ComplexNS> odd(n / 2);
    for (int i = 0; i < n / 2; i++) {
        even[i] = input[2 * i];
        odd[i] = input[2 * i + 1];
    }

    vector<ComplexNS> even_fft = recursive_fft(even);
    vector<ComplexNS> odd_fft = recursive_fft(odd);

    vector<ComplexNS> output(n);
    for (int k = 0; k < n / 2; k++) {
        double angle = -2 * M_PI * k / n;
        ComplexNS factor = exp_complex(angle);
        output[k] = even_fft[k] + factor * odd_fft[k];
        output[k + n / 2] = even_fft[k] - factor * odd_fft[k];
    }

    return output;
}

// Обратное одномерное БПФ
vector<ComplexNS> inverse_fft(const vector<ComplexNS>& input) {
    int n = input.size();
    vector<ComplexNS> output = recursive_fft(input);
    for (int i = 0; i < n; i++) {
        output[i] = output[i] * (1.0 / n);
    }
    return output;
}

// Двумерное БПФ с помощью одномерного БПФ
vector<vector<ComplexNS>> fft2D(const vector<vector<double>>& input) {
    int rows = input.size();
    int cols = input[0].size();

    // Преобразование в комплексный формат
    vector<vector<ComplexNS>> complex_image(rows, vector<ComplexNS>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            complex_image[i][j] = ComplexNS(input[i][j], 0);
        }
    }

    // БПФ по строкам
    for (int i = 0; i < rows; i++) {
        complex_image[i] = recursive_fft(complex_image[i]);
    }

    // БПФ по столбцам
    for (int j = 0; j < cols; j++) {
        vector<ComplexNS> column(rows);
        for (int i = 0; i < rows; i++) {
            column[i] = complex_image[i][j];
        }
        column = recursive_fft(column);
        for (int i = 0; i < rows; i++) {
            complex_image[i][j] = column[i];
        }
    }

    return complex_image;
}

// Обратное двумерное БПФ
vector<vector<double>> inverse_fft2D(const vector<vector<ComplexNS>>& F) {
    int rows = F.size();
    int cols = F[0].size();

    // Обратное БПФ по столбцам
    vector<vector<ComplexNS>> inverse_F(rows, vector<ComplexNS>(cols));
    for (int j = 0; j < cols; j++) {
        vector<ComplexNS> column(rows);
        for (int i = 0; i < rows; i++) {
            column[i] = F[i][j];
        }
        column = inverse_fft(column);
        for (int i = 0; i < rows; i++) {
            inverse_F[i][j] = column[i];
        }
    }

    // Обратное БПФ по строкам
    for (int i = 0; i < rows; i++) {
        inverse_F[i] = inverse_fft(inverse_F[i]);
    }

    // Преобразование в вещественный формат
    vector<vector<double>> inverse_image(rows, vector<double>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            inverse_image[i][j] = inverse_F[i][j].real;
        }
    }

    return inverse_image;
}

int main() 
{
    const char* fullPath = "C:/sobel_2.jpg";
    cv::Mat original = cv::imread(fullPath, cv::IMREAD_GRAYSCALE);

    const int ROWS = original.rows;
    const int COLS = original.cols;

    cv::Mat fft = cv::Mat(ROWS, COLS, cv::IMREAD_GRAYSCALE);

    vector<vector<double>> image(ROWS, vector<double>(COLS));
    for (int i = 0; i < ROWS; ++i) {
        uchar* pixels = original.ptr<uchar>(i); 
        for (int j = 0; j < COLS; ++j) {
            image[i][j] = static_cast<double>(pixels[j]);
        }
    }
    
    // Вычисление БПФ
    vector<vector<ComplexNS>> F = fft2D(image);

    // Вычисление обратного БПФ
    vector<vector<double>> inverse_image = inverse_fft2D(F);

    // Вывод результата
    cout << "Обратное БПФ:" << endl;
    for (int i = 0; i < inverse_image.size(); i++) {
        uchar* pixles = fft.ptr<uchar>(i);
        for (int j = 0; j < inverse_image[0].size(); j++) {
            //cout << inverse_image[i][j] << " ";
            pixles[j] = static_cast<uchar>(inverse_image[i][j]);
        }
        //cout << endl << "new line" << endl;
    }

    cv::imshow("awda", fft);
    cv::waitKey(0);

    return 0;
}