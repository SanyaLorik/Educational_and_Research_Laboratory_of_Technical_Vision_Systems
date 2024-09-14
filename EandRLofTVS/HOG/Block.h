#pragma once

#include <cstring>
#include <iostream>

using namespace std;

class Block
{

public:
    const int SIZE_IN_CELL = 9;

    float* histogram;
    int size = 0;
    int index = 0;

    Block() : size(0), index(0), histogram(nullptr) {}

    Block(int size);

    Block(const Block& other);

    ~Block();

    Block& operator=(const Block& other);

    void Add(int* insert);

    float* NormalizeHistogram();

    int CalculateSize();
};