#include "Lpq.h"

void Lpq::calculalte_fft2d()
{
    transformToComplex();
    fft2D();
}

void Lpq::calculalte_inverse_fft2D()
{
    inverse_fft2D();
}

void Lpq::transformToComplex()
{
    _forward = vector<vector<ComplexNS>>(rows, vector<ComplexNS>(cols));
    for (int y = 0; y < rows; y++) {
        uchar* pixels = original.ptr<uchar>(y);
        for (int x = 0; x < cols; x++)
            _forward[y][x] = ComplexNS(pixels[x], 0.0);
    }
}

void Lpq::fft2D()
{
    // БПФ по строкам
    for (int y = 0; y < rows; y++)
        _forward[y] = recursive_fft(_forward[y]);

    // БПФ по столбцам
    for (int x = 0; x < cols; x++) {

        vector<ComplexNS> column(rows);
        for (int y = 0; y < rows; y++)
            column[y] = _forward[y][x];

        column = recursive_fft(column);
        for (int y = 0; y < rows; y++)
            _forward[y][x] = column[y];
    }
}

void Lpq::inverse_fft2D() {

    // Обратное БПФ по столбцам
    vector<vector<ComplexNS>> inverse_F(rows, vector<ComplexNS>(cols));
    for (int j = 0; j < cols; j++)
    {
        vector<ComplexNS> column(rows);
        for (int i = 0; i < rows; i++)
            column[i] = _forward[i][j];

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

vector<ComplexNS> Lpq::inverse_fft(const vector<ComplexNS>& input) 
{
    int n = input.size();
    vector<ComplexNS> output = recursive_fft(input);

    for (int i = 0; i < n; i++)
        output[i] = output[i] * (1.0 / n);

    return output;
}

void Lpq::transformToReal(const vector<vector<ComplexNS>>& inverse_F)
{
    _invers = vector<vector<uchar>>(rows, vector<uchar>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            _invers[i][j] = inverse_F[i][j].real;
    }
}

vector<int>* Lpq::CalculateHistogram(int kernelSize, int step)
{
    vector<int>* histogram = new vector<int>(SEGMENT, 0);

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

vector<vector<ComplexNS>> Lpq::get_window(int y, int x, int kernelSize)
{
    vector<vector<ComplexNS>> window = vector<vector<ComplexNS>>(kernelSize, vector<ComplexNS>(kernelSize));

    int half = kernelSize / 2;

    for (int yk = -half; yk < half; yk++)
    {
        uchar* pixels = original.ptr<uchar>(y + yk);
        for (int xk = -half; xk < half; xk++)
            window[yk + half][xk + half] = ComplexNS(pixels[x + xk], 0.0);
    }

    return window;
}

void Lpq::fft2D(vector<vector<ComplexNS>>& window, int kernelSize)
{
    // БПФ по строкам
    for (int i = 0; i < kernelSize; i++)
        window[i] = recursive_fft(window[i]);

    // БПФ по столбцам
    for (int j = 0; j < kernelSize; j++) 
    {
        vector<ComplexNS> column(kernelSize);
        for (int i = 0; i < kernelSize; i++)
            column[i] = window[i][j];

        column = recursive_fft(column);
        for (int i = 0; i < kernelSize; i++)
            window[i][j] = column[i];
    }
}

vector<int> Lpq::get_local_histogram(const vector<vector<ComplexNS>>& fft2d)
{
    vector<int> histogram(SEGMENT, 0);
    const float ratio = 2.0f * M_PI / SEGMENT;

    for (int y = 0; y < fft2d.size(); y++)
    {
        for (int x = 0; x < fft2d[y].size(); x++)
        {
            float phase = fft2d[y][x].phase();
            int index = (int)(fmod(phase / ratio + SEGMENT, SEGMENT));
            histogram[index]++;
        }
    }

    return histogram;
}

void Lpq::sum_locals_histograms(vector<int>& histogram, const vector<vector<int>>& locals)
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

vector<ComplexNS> Lpq::recursive_fft(const vector<ComplexNS>& input) {
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