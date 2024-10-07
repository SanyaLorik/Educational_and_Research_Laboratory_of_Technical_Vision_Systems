#include "HistogramValue.h"

HistogramValue::HistogramValue(short value) : Value(value) {}

void HistogramValue::add() 
{
    Count++;
}