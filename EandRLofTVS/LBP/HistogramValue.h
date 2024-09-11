#pragma once

#include <cstdint>

struct HistogramValue 
{

public:
    uint8_t Value;
    short Count = 1;

    HistogramValue() = default;

    HistogramValue(short value);

    void Add();
};