#pragma once

// ComplexNS -> Complex Nothing Superfluous (ничего лишнего)
struct ComplexNS
{
	double real;
	double imag;

	ComplexNS(double r = 0, double i = 0) : real(r), imag(i) {}

	ComplexNS operator+(const ComplexNS& c) const;

	ComplexNS operator-(const ComplexNS& c) const;

	ComplexNS operator*(const ComplexNS& c) const;

	ComplexNS operator*(const double& c) const;

	ComplexNS conjugate() const;

	double phase() const;
}; 

ComplexNS exp_complex(double angle);