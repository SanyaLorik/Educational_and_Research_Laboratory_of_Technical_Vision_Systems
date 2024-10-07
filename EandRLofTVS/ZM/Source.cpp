#include <iostream>
#include <complex>
#include <cmath>
#include <vector>

using namespace std;
const double M_PI = 3.1415926535897;

// ��������� ��� ������������� ������������ �����
struct Complex {
    double real;
    double imag;

    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    Complex operator+(const Complex& c) const {
        return Complex(real + c.real, imag + c.imag);
    }

    Complex operator-(const Complex& c) const {
        return Complex(real - c.real, imag - c.imag);
    }

    Complex operator*(const Complex& c) const {
        return Complex(real * c.real - imag * c.imag, real * c.imag + imag * c.real);
    }

    Complex operator*(const double& c) const {
        return Complex(real * c, imag * c);
    }

    Complex conjugate() const {
        return Complex(real, -imag);
    }
};

// ����� ������������ �����
ostream& operator<<(ostream& out, const Complex& c) {
    out << c.real << " + " << c.imag << "i";
    return out;
}

// ���������� ���������� ������������ �����
Complex exp_complex(double angle) {
    return Complex(cos(angle), sin(angle));
}

// ����������� ���������� ��� ����������� �������
vector<Complex> recursive_fft(const vector<Complex>& input) {
    int n = input.size();
    if (n == 1) {
        return { input[0] };
    }

    vector<Complex> even(n / 2);
    vector<Complex> odd(n / 2);
    for (int i = 0; i < n / 2; i++) {
        even[i] = input[2 * i];
        odd[i] = input[2 * i + 1];
    }

    vector<Complex> even_fft = recursive_fft(even);
    vector<Complex> odd_fft = recursive_fft(odd);

    vector<Complex> output(n);
    for (int k = 0; k < n / 2; k++) {
        double angle = -2 * M_PI * k / n;
        Complex factor = exp_complex(angle);
        output[k] = even_fft[k] + factor * odd_fft[k];
        output[k + n / 2] = even_fft[k] - factor * odd_fft[k];
    }

    return output;
}

// �������� ���������� ���
vector<Complex> inverse_fft(const vector<Complex>& input) {
    int n = input.size();
    vector<Complex> output = recursive_fft(input);
    for (int i = 0; i < n; i++) {
        output[i] = output[i] * (1.0 / n);
    }
    return output;
}

// ��������� ��� � ������� ����������� ���
vector<vector<Complex>> fft2D(const vector<vector<double>>& input) {
    int rows = input.size();
    int cols = input[0].size();

    // �������������� � ����������� ������
    vector<vector<Complex>> complex_image(rows, vector<Complex>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            complex_image[i][j] = Complex(input[i][j], 0);
        }
    }

    // ��� �� �������
    for (int i = 0; i < rows; i++) {
        complex_image[i] = recursive_fft(complex_image[i]);
    }

    // ��� �� ��������
    for (int j = 0; j < cols; j++) {
        vector<Complex> column(rows);
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

// �������� ��������� ���
vector<vector<double>> inverse_fft2D(const vector<vector<Complex>>& F) {
    int rows = F.size();
    int cols = F[0].size();

    // �������� ��� �� ��������
    vector<vector<Complex>> inverse_F(rows, vector<Complex>(cols));
    for (int j = 0; j < cols; j++) {
        vector<Complex> column(rows);
        for (int i = 0; i < rows; i++) {
            column[i] = F[i][j];
        }
        column = inverse_fft(column);
        for (int i = 0; i < rows; i++) {
            inverse_F[i][j] = column[i];
        }
    }

    // �������� ��� �� �������
    for (int i = 0; i < rows; i++) {
        inverse_F[i] = inverse_fft(inverse_F[i]);
    }

    // �������������� � ������������ ������
    vector<vector<double>> inverse_image(rows, vector<double>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            inverse_image[i][j] = inverse_F[i][j].real;
        }
    }

    return inverse_image;
}

int main() {
    // �������� ����������� (�������)
    vector<vector<double>> image = {
        { 107, 138, 123 },
        { 103, 101, 108 },
        { 152, 98, 96 },
    };

    // ���������� ���
    vector<vector<Complex>> F = fft2D(image);   
    cout << "�������� ���:" << endl;
    for (int i = 0; i < F.size(); i++) {
        for (int j = 0; j < F[0].size(); j++) {
            cout << F[i][j] << " ";
        }
        cout << endl;
    }

    // ���������� ��������� ���
    vector<vector<double>> inverse_image = inverse_fft2D(F);

    // ����� ����������
    /*
    cout << "�������� ���:" << endl;
    for (int i = 0; i < inverse_image.size(); i++) {
        for (int j = 0; j < inverse_image[0].size(); j++) {
            cout << inverse_image[i][j] << " ";
        }
        cout << endl;
    }
    */
    return 0;
}