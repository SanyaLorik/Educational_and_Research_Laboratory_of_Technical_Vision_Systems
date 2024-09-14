#include "Block.h"

Block::Block(int size) : size(size)
{
    histogram = new float[size * size * SIZE_IN_CELL];
}

Block::Block(const Block& other) : size(other.size), index(other.index)
{
    histogram = new float[CalculateSize()];
    memcpy(histogram, other.histogram, CalculateSize() * sizeof(float));
}

Block::~Block()
{
    delete[] histogram;
}

Block& Block::operator=(const Block& other)
{
    if (this != &other) // Защита от самоприсваивания
    {
        delete[] histogram; // Освобождаем старую память

        size = other.size;
        index = other.index;
        histogram = new float[CalculateSize()];
        memcpy(histogram, other.histogram, CalculateSize() * sizeof(float));
    }

    return *this;
}

void Block::Add(int* insert)
{
    int indexTemp = 0;
    for (size_t i = index; i < index + SIZE_IN_CELL; i++, indexTemp++)
        histogram[i] = insert[indexTemp];

    index += SIZE_IN_CELL;
}

float* Block::NormalizeHistogram()
{
    float sum = 0;
    int size = CalculateSize();

    for (int i = 0; i < size; i++)
        sum += histogram[i];


    for (int i = 0; i < size; i++)
        histogram[i] /= sum;

    return histogram;
}

int Block::CalculateSize()
{
    return size * size * SIZE_IN_CELL;
}