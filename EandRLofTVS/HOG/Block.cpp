#include "Block.h"

Block::Block(int size) : Size(size)
{
    Histogram = new float[size * size * SIZE_IN_CELL];
}

Block::Block(const Block& other) : Size(other.Size), Index(other.Index)
{
    Histogram = new float[CalculateSize()];
    memcpy(Histogram, other.Histogram, CalculateSize() * sizeof(float));
}

Block::~Block()
{
    delete[] Histogram;
}

Block& Block::operator=(const Block& other)
{
    if (this != &other) // Защита от самоприсваивания
    {
        delete[] Histogram; // Освобождаем старую память

        Size = other.Size;
        Index = other.Index;
        Histogram = new float[CalculateSize()];
        memcpy(Histogram, other.Histogram, CalculateSize() * sizeof(float));
    }

    return *this;
}

void Block::Add(int* insert)
{
    int indexTemp = 0;
    for (size_t i = Index; i < Index + SIZE_IN_CELL; i++, indexTemp++)
        Histogram[i] = insert[indexTemp];

    Index += SIZE_IN_CELL;
}

float* Block::NormalizeHistogram()
{
    float sum = 0;
    int size = CalculateSize();

    for (int i = 0; i < size; i++)
        sum += Histogram[i];


    for (int i = 0; i < size; i++)
        Histogram[i] /= sum;

    return Histogram;
}

int Block::CalculateSize()
{
    return Size * Size * SIZE_IN_CELL;
}