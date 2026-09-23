/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw14_04
* Description: 复数类模板
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/4/2 13:27:59
*/
#include <iostream>
#include <cmath>

template<typename T>
class Complex {
private:
	T real;
	T imag;
public:
	Complex():real(0), imag(0) {}
	Complex(T r, T i):real(r), imag(i) {}
	Complex(const T& other):real(other.r), imag(other.i) {}
	virtual ~Complex() = default;

	double Magnitude() const;
	Complex operator+(const Complex& other) const;
	void Display() const;
};

template<typename T>
double Complex<T>::Magnitude() const {
	return std::sqrt(real * real + imag * imag);
}

template<typename T>
Complex<T> Complex<T>::operator+(const Complex<T>& other) const {
	return Complex<T>(real + other.real, imag + other.imag);
}

template<typename T>
void Complex<T>::Display() const {
	if (real == 0 && imag == 0) {
		std::cout << "0";
		return;
	}
	if (real != 0) {
		std::cout << real;
	}
	if (imag != 0) {
		if (imag > 0 && real != 0) std::cout << "+";
		if (imag == 1) {
			std::cout << "i";
		}
		else if (imag == -1) {
			std::cout << "-i";
		}
		else {
			std::cout << imag << "i";
		}
	}
}

int main() {
	Complex<float> z1(2.1f, 5.3f);
	Complex<float> z2(1.9f, -2.3f);
	Complex<float> sum_float = z1 + z2;
	std::cout << "z1 + z2 = ";
	sum_float.Display();
	std::cout << std::endl;
	std::cout << "Magnitude of z1 + z2 = " << sum_float.Magnitude() << std::endl;

	Complex<int> a1(2, 5);
	Complex<int> a2(1, -2);
	Complex<int> sum_int = a1 + a2;
	std::cout << "a1 + a2 = ";
	sum_int.Display();
	std::cout << std::endl;
	std::cout << "Magnitude of a1 + a2 = " << sum_int.Magnitude() << std::endl;

	return 0;
}