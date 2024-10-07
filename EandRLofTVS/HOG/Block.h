#pragma once

#include <cstring>
#include <iostream>

using namespace std;

class Block
{

public:
    const int SIZE_IN_CELL = 9;

    float* Histogram;
    int Size = 0;
    int Index = 0;

    Block() : Size(0), Index(0), Histogram(nullptr) {}

    Block(int size);

    Block(const Block& other);

    ~Block();

    Block& operator=(const Block& other);

    void Add(int* insert);

    float* NormalizeHistogram();

    int CalculateSize();
};