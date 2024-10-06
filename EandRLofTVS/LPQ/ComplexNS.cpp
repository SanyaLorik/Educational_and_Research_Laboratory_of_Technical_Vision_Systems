#include "ComplexNS.h"

#include <cmath>

ComplexNS  ComplexNS::operator+(const ComplexNS& c) const 
{
	return ComplexNS(real + c.real, imag + c.imag);
}

ComplexNS  ComplexNS::operator-(const ComplexNS& c) const 
{
    return ComplexNS(real - c.real, imag - c.imag);
}

ComplexNS  ComplexNS::operator*(const ComplexNS& c) const
{
    return ComplexNS(real * c.real - imag * c.imag, real * c.imag + imag * c.real);
}

ComplexNS  ComplexNS::operator*(const double& c) const 
{
    return ComplexNS(real * c, imag * c);
}

ComplexNS  ComplexNS::conjugate() const
{
    return ComplexNS(real, -imag);
}

ComplexNS exp_complex(double angle)
{
    return ComplexNS(cos(angle), sin(angle));
}