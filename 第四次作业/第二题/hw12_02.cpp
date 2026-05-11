/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw12_02
* Description:复数类，重载+运算符，支持复数与复数、复数与实数的加法运算
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/24 19:23:32
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
	Complex(float x) :x(x), y(0) {}
	Complex(const Complex& other) :x(other.x), y(other.y) {};
	virtual ~Complex() = default;

	void display() const;
	friend Complex operator+(const Complex& a, const Complex& b);
private:
	float x, y;
};

void Complex::display() const {
	if (isEqual(x, 0) && isEqual(y, 0)) {
		cout << 0;
	}
	else if (isEqual(x, 0)) {
		if (isEqual(y, 1)) {
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

Complex operator+(const Complex& a, const Complex& b) {
	Complex res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	return res;
}

int main() {
	Complex a(2.1, 5.7), b(7.5, 8), c, d, e;
	c = a + b;
	d = b + 5.6;
	e = 4.1 + a;
	c.display();
	d.display();
	e.display();
	return 0;
}