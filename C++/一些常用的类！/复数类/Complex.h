#pragma once

#include <iostream>

const double EPSILON = 1e-9;

inline static bool isEqual(double a, double b) {
	return abs(a - b) < EPSILON;
}

class complex {
	double real, imag;
public:
	complex(int real = 0.0, int imag = 0.0) :real(real), imag(imag) {}
	complex(const complex& other) :real(other.real), imag(other.imag) {}
	virtual ~complex() = default;

	complex operator=(const complex& other);

	friend complex operator+(const complex&, const complex&);
	friend complex operator-(const complex&, const complex&);
	friend complex operator*(const complex&, const complex&);
	friend complex operator/(const complex&, const complex&);

	void display() const;
	friend std::ostream& operator<<(std::ostream& os, const complex& out);
	friend std::istream& operator>>(std::istream& is, const complex& in);
	double complexMod() const;
	double getRealPart() const { return this->real; }
	double getImagPart() const { return this->imag; }
};

complex complex::operator=(const complex& other) {
	if (this != &other) {
		real = other.real;
		imag = other.imag;
	}
	return *this;
}

complex operator+(const complex& other1, const complex& other2) {
	complex res;
	res.real = other1.real + other2.real;
	res.imag = other1.imag + other2.imag;
	return res;
}

complex operator-(const complex& other1, const complex& other2) {
	complex res;
	res.real = other1.real - other2.real;
	res.imag = other1.imag - other2.imag;
	return res;
}

complex operator*(const complex& other1, const complex& other2) {
	complex res;
	res.real = other1.real * other2.real - other1.imag * other2.imag;
	res.imag = other1.real * other2.imag + other1.imag * other2.real;
	return res;
}

complex operator/(const complex& other1, const complex& other2) {
	complex res;
	res.real = (other1.real * other2.real + other1.imag + other2.imag) / (other2.real * other2.real + other2.imag * other2.imag);
	res.imag = (other1.imag * other2.real - other1.real * other2.imag) / (other2.real * other2.real + other2.imag * other2.imag);
	return res;
}

void complex::display() const {
	if (isEqual(real, 0) && isEqual(imag, 0)) std::cout << 0;
	else if (isEqual(real, 0)) {
		if (isEqual(abs(imag), 1)) std::cout << (imag > 0 ? "i" : "-i");
		else std::cout << imag << "i";
	}
	else if (isEqual(imag, 0)) std::cout << real;
	else {
		std::cout << real;
		if (imag > 0) std::cout << "+";
		std::cout << imag << "i";
	}
	std::cout << std::endl;
}

inline std::ostream& operator<<(std::ostream& os, const complex& out) {
	double real = c.real;
	double imag = c.imag;
	if (isEqual(real, double(0)) && isEqual(imag, double(0))) {
		os << 0;
	}
	else if (isEqual(real, double(0))) {
		if (isEqual(std::abs(imag), double(1)))
			os << (imag > 0 ? "i" : "-i");
		else
			os << imag << "i";
	}
	else if (isEqual(imag, double(0))) {
		os << real;
	}
	else {
		os << real;
		if (imag > 0) os << "+";
		os << imag << "i";
	}
	return os;
}

inline double complex::complexMod() const {
	return sqrt(this->real * this->real + this->imag * this->imag);
}