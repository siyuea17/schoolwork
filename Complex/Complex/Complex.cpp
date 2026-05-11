#include "pch.h"
#include "Complex.h"
#include <iostream>

Complex::Complex() :x(0), y(0) {}
Complex::Complex(float x, float y) :x(x), y(y) {}
void Complex::Display() {
	if (x == 0 && y != 0) {
		std::cout << this->y << "i" << std::endl;
	}
	else if (x == 0 && y == 0) {
		std::cout << "0" << std::endl;
	}
	else if (x != 0 && y == 0) {
		std::cout << this->x << std::endl;
	}
	else if (x != 0 && y > 0) {
		std::cout << this->x << "+" << this->y << "i" << std::endl;
	}
	else {
		std::cout << this->x << this->y << "i" << std::endl;
	}

}
Complex Complex::operator+=(Complex& p) {
	this->x += p.x;
	this->y += p.y;
	Complex r(this->x, this->y);
	return r;
}
Complex operator+(Complex& p1, Complex& p2) {
	Complex p3(p1.x + p2.x, p1.y + p2.y);
	return p3;
}
Complex operator+(Complex& p, float n) {
	Complex p3(p.x + n, p.y);
	return p3;
}
Complex operator+(float n, Complex& p) {
	Complex p3(p.x + n, p.y);
	return p3;
}
Complex operator/(Complex& p1, Complex& p2) {
	Complex fp1(p1.x, -p1.y);
	Complex fp2(p2.x, -p2.y);
	float nx = p1.x * p2.x + p1.y * p2.y;
	float ny = p2.x * p1.y - p1.x * p2.y;
	float fm = p2.x * p2.x + p2.y * p2.y;
	Complex r(nx / fm, ny / fm);
	return r;
}