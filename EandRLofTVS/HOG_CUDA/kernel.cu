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

/*
__global__ void gauss(
    const uchar* input_image, uchar* output_image, float* kernel, 
    int kernelSize, int width_image, int height_image)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width_image || y >= height_image)
        return;

    int centre = y * width_image + x;
    float sumColor = 0;

    for (int kx = 0; kx < kernelSize; kx++)
    {
        for (int ky = 0; ky < kernelSize; ky++)
        {
            if ((x + kx) >= 0 && (x + kx) < width_image && (y + ky) >= 0 && (y + ky) < height_image)
            {
                int offset = (y + ky) * width_image + (x + kx);

                float maskValue = kernel[ky * kernelSize + kx];
                sumColor += input_image[offset] * maskValue;
            }
        }
    }

    output_image[centre] = (uchar)sumColor;
}
*/

__global__ void gauss(
    const cudaTextureObject_t input_image, uchar* output_image, float* kernel,
    int kernelSize, int width_image, int height_image)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    float sumColor = 0;

    for (int kx = 0; kx < kernelSize; kx++)
    {
        for (int ky = 0; ky < kernelSize; ky++)
        {
            float maskValue = kernel[ky * kernelSize + kx];
            sumColor += tex2D<uchar>(input_image, x + kx, y + ky) * maskValue;
        }
    }

    int centre = y * width_image + x;
    output_image[centre] = (uchar)sumColor;
}

float* createGaussianKernel(int kernelSize, double sigma);

float guassian(int x, int y, double sigma);

int main()
{
    Mat image = imread("C:/sobel.jpg", IMREAD_GRAYSCALE);

    int kernelSize = 11;
    double sigma = 8;
    float* kernel = createGaussianKernel(kernelSize, sigma);

    int block_y = (image.rows + 31) / 32;
    int block_x = (image.cols + 31) / 32;

    uchar* d_input_image;
    cudaMalloc(&d_input_image, image.total() * sizeof(uchar));
    cudaMemcpy(d_input_image, image.data, image.total() * sizeof(uchar), cudaMemcpyHostToDevice);

    uchar* d_output_image;
    cudaMalloc(&d_output_image, image.total() * sizeof(uchar));

    float* d_kernel;
    cudaMalloc(&d_kernel, kernelSize * kernelSize * sizeof(float));
    cudaMemcpy(d_kernel, kernel, kernelSize * kernelSize * sizeof(float), cudaMemcpyHostToDevice);

    cudaTextureObject_t texObj;

    cudaArray* d_array;
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<uchar>();
    cudaMallocArray(&d_array, &channelDesc, image.cols, image.rows);
    
    cudaMemcpyToArray(d_array, 0, 0, image.data, image.total() * sizeof(uchar), cudaMemcpyHostToDevice);

    cudaResourceDesc resDesc = {};
    resDesc.resType = cudaResourceTypeArray;
    resDesc.res.array.array = d_array;

    cudaTextureDesc texDesc = {};
    texDesc.addressMode[0] = cudaAddressModeClamp;
    texDesc.addressMode[1] = cudaAddressModeClamp;
    texDesc.filterMode = cudaFilterModePoint;
    texDesc.readMode = cudaReadModeElementType;
    texDesc.normalizedCoords = 0;

    cudaCreateTextureObject(&texObj, &resDesc, &texDesc, nullptr);

    gauss <<<dim3(block_x, block_y), dim3(32, 32)>>> (
        texObj, d_output_image, d_kernel,
        kernelSize, image.cols, image.rows);

    uchar* h_output_image = new uchar[image.total()];

    cudaMemcpy(h_output_image, d_output_image, image.total() * sizeof(uchar), cudaMemcpyDeviceToHost);
    
    Mat output_image = Mat(image.size(), CV_8UC1);
    for (int y = 0; y < image.rows; y++)
    {
        uchar* pixels = output_image.ptr<uchar>(y);
        for (int x = 0; x < image.cols; x++)
            pixels[x] = h_output_image[y * image.cols + x];
    }

    cudaFree(d_input_image);
    cudaFree(d_output_image);
    cudaFree(d_kernel);

    delete[] h_output_image;

    cv::imshow("image", output_image);
    cv::waitKey(0);

    return 0;
}

float* createGaussianKernel(int kernelSize, double sigma) 
{
    float* kernel = new float[kernelSize * kernelSize];

    int indent = kernelSize / 2;
    float sum = 0.0f;

    for (int y = -indent; y <= indent; y++)
    {
        for (int x = -indent; x <= indent; x++) 
        {
            float value = guassian(x, y, sigma);
            sum += value;
            kernel[(y + indent) * kernelSize + (x + indent)] = value;
        }
    }

    for (int i = 0; i < kernelSize * kernelSize; i++) 
        kernel[i] /= sum;

    return kernel;
}

float guassian(int x, int y, double sigma)
{
    return (1 / (2 * 3.1415f * powf(sigma, 2))) * expf(-((x * x + y * y) / (2 * powf(sigma, 2))));
}