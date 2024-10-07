#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <cuda_runtime.h>
#include <cuda_fp16.h>

#include <iostream>

using namespace std;
using namespace cv;

// Ядро CUDA для вычисления LBP
__global__ void calculate_lbp(const uchar* input_image, int* output_codes, int* histogram, int width_image, int height_image, int radius_neighbors, int count_neighbors)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x; // Координата x текущего пикселя
    int y = blockIdx.y * blockDim.y + threadIdx.y; // Координата y текущего пикселя

    if (x >= width_image || y >= height_image)
        return;

    double const M_PI = 3.1415926535f;

    int center = y * width_image + x;

    unsigned char code = 0;
    for (int n = 0; n < count_neighbors; n++)
    {
        double angle = 2 * M_PI * n / count_neighbors;
        int neighbor_x = x + radius_neighbors * cos(angle);
        int neighbor_y = y + radius_neighbors * sin(angle);

        if (neighbor_x >= 0 && neighbor_x < width_image && neighbor_y >= 0 && neighbor_y < height_image)
        {
            if (input_image[neighbor_y * width_image + neighbor_x] >= input_image[center])
                code |= (1 << n);
        }
    }

    output_codes[center] = code;

    int* number = &(histogram[code]);
    atomicAdd(number, 1);
}

int main()
{
    Mat image = imread("C:/sobel.jpg", IMREAD_GRAYSCALE);

    int radius_neighbors = 1;
    int count_neighbors = 8; 

    int blocks_x = (image.cols + 31) / 32; 
    int blocks_y = (image.rows + 31) / 32;

    uchar* d_input;
    cudaMalloc(&d_input, image.total() * sizeof(uchar));
    cudaMemcpy(d_input, image.data, image.total() * sizeof(uchar), cudaMemcpyHostToDevice);

    int* d_output;
    cudaMalloc(&d_output, image.total() * sizeof(int));

    int size_histogram = 256;
    int* d_histogram;
    cudaMalloc(&d_histogram, size_histogram * sizeof(int));

    calculate_lbp <<<dim3(blocks_x, blocks_y), dim3(32, 32)>>>(d_input, d_output, d_histogram, image.cols, image.rows, radius_neighbors, count_neighbors);

    int* h_output = new int[image.total()];
    int* h_histogram = new int[size_histogram];

    cudaMemcpy(h_output, d_output, image.total() * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_histogram, d_histogram, size_histogram * sizeof(int), cudaMemcpyDeviceToHost);

    Mat output_image = Mat(image.size(), CV_8UC1);
    for (int y = 0; y < image.rows; y++)
    {
        uchar* pixels = output_image.ptr<uchar>(y);
        for (int x = 0; x < image.cols; x++)
            pixels[x] = h_output[y * image.cols + x];
    }

    cudaFree(d_input);
    cudaFree(d_output);

    delete[] h_histogram;
    delete[] h_output;

    cv::imshow("image", output_image);
    cv::waitKey(0);

    return 0;
}