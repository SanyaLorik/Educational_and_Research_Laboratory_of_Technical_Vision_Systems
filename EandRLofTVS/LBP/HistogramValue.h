#include <iostream>

using namespace std;

struct HistogramValue
{

public:
	uint8_t Value;
	short Count = 1;

	HistogramValue() = default;

	HistogramValue(short value) : Value(value)
	{

	}

	void Add()
	{
		Count++;
	}
};