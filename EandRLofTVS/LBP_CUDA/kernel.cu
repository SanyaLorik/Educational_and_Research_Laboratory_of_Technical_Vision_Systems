#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <texture_fetch_functions.h> 
#include <cuda_fp16.h>
#include <cuda.h>

#include <iostream>

using namespace std;
using namespace cv;

__global__ void calculate_lbp(const uchar* input_image, int* output_codes, int* histogram, int width_image, int height_image, int radius_neighbors, int count_neighbors)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x; // текущего пикселя
    int y = blockIdx.y * blockDim.y + threadIdx.y; // текущего пикселя

    if (x >= width_image || y >= height_image)
        return;

    double const M_PI = 3.1415926535f;

    int center = y * width_image + x;

    unsigned char code = 0;
    for (int n = 0; n < count_neighbors; n++)
    {
        double angle = 2 * M_PI * n / count_neighbors;
        int neighbor_x = x + radius_neighbors * nearbyint(cos(angle));
        int neighbor_y = y + radius_neighbors * nearbyint(sin(angle));

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

__global__ void gauss(const uchar* input_image, uchar* output_iamge, const float** kernel, int kernelSize, int width_image, int height_image)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width_image || y >= height_image)
        return;

    int centre = y * width_image + x;
    int sumColor = 0;

    for (int kx = 0; kx < kernelSize; kx++)
    {
        for (int ky = 0; ky < kernelSize; ky++)
        {
            if ((x + kx) >= 0 && (x + kx) < width_image && (y + ky) >= 0 && (y + ky) < height_image)
            {
                int offset = (y + ky) * width_image + (x + kx);

                float maskValue = kernel[ky][kx];
                sumColor += input_image[offset] * maskValue;
            }
        }
    }

    output_iamge[centre] = sumColor;
}

int main()
{
    Mat image = imread("C:/sobel.jpg", IMREAD_GRAYSCALE);

    int radius_neighbors = 1;
    int count_neighbors = 8;

    int blocks_x = (image.cols + 31) / 32;
    int blocks_y = (image.rows + 31) / 32;

    uchar* d_input_image;
    cudaMalloc(&d_input_image, image.total() * sizeof(uchar));
    cudaMemcpy(d_input_image, image.data, image.total() * sizeof(uchar), cudaMemcpyHostToDevice);

    int* d_output_code;
    cudaMalloc(&d_output_code, image.total() * sizeof(int));

    int size_histogram = 256;
    int* d_histogram;
    cudaMalloc(&d_histogram, size_histogram * sizeof(int));

    calculate_lbp << <dim3(blocks_x, blocks_y), dim3(32, 32) >> > (
        d_input_image, d_output_code, d_histogram, 
        image.cols, image.rows, radius_neighbors, count_neighbors);

    int* h_output_code = new int[image.total()];
    int* h_histogram = new int[size_histogram];

    cudaMemcpy(h_output_code, d_output_code, image.total() * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_histogram, d_histogram, size_histogram * sizeof(int), cudaMemcpyDeviceToHost);

    Mat output_image = Mat(image.size(), CV_8UC1);
    for (int y = 0; y < image.rows; y++)
    {
        uchar* pixels = output_image.ptr<uchar>(y);
        for (int x = 0; x < image.cols; x++)
            pixels[x] = h_output_code[y * image.cols + x];
    }

    cudaFree(d_input_image);
    cudaFree(d_output_code);

    delete[] h_histogram;
    delete[] h_output_code;

    cv::imshow("image", output_image);
    cv::waitKey(0);

    return 0;
}