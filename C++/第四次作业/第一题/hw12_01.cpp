/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw12_01
* Description:复数类，重载/和+=运算符
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/24 18:49:28
*/

#include <iostream>
#include <cmath>

using namespace std;

const float EPSILON = 1e-6;

inline static bool isEqual(float a, float b) {
	return abs(a - b) < EPSILON;
}

class Complex {
public:
    Complex() :x(0), y(0) {}
    Complex(float x, float y) :x(x), y(y) {}
    Complex(const Complex& other) :x(other.x), y(other.y) {};
    virtual ~Complex() = default;
	void display() const;
    Complex operator+=(const Complex&);
    friend Complex operator/(const Complex& a, const Complex& b);
private:
	float x, y;
};

void Complex::display() const {
	if (isEqual(x,0) && isEqual(y,0)) {
		cout << 0;
	}
	else if (isEqual(x, 0)) {
		if (isEqual(abs(y),1)) {
			cout << (y > 0 ? "i" : "-i");
		}
		else {
			cout << y << "i";
		}
	}
	else if (isEqual(y, 0)) {
		cout << x;
	}
	else {
		cout << x;
		if (y > 0) cout << "+";
		cout << y << "i";
	}
	cout << endl;
}

Complex Complex::operator+=(const Complex& other) {
	this->x += other.x;
	this->y += other.y;
	return *this;
}

Complex operator/(const Complex& a, const Complex& b) {
	Complex res;
	res.x = (a.x * b.x + a.y + b.y) / (b.x * b.x + b.y * b.y);
	res.y = (a.y * b.x - a.x * b.y) / (b.x * b.x + b.y * b.y);
	return res;
}

int main() {
	Complex z1(2.1, -5.4), z2(7.5, 3.2);
	Complex z3 = z2 / z1;
	z3.display();
	z3 += z1;
	z3.display();
    return 0;
}

