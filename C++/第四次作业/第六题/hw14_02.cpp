/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw14_02
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/26 20:01:28
*/

#include <iostream>
#include <cmath>
#include <stdexcept>

using namespace std;

int gcd(int a, int b) {
	a = abs(a);
	b = abs(b);
	while (b != 0) {
		int temp = b;
		b = a % b;
		a = temp;
	}
	return a;
}

class Integer {
protected:
	int x;
public:
	Integer() :x(0) {}
	Integer(int x) :x(x) {}
	virtual ~Integer() = default;
	Integer(const Integer& other) :x(other.x) {}
	virtual void Display() const = 0;
};

class Rational :public Integer {
private:
	int y;
public:
	Rational() :Integer(0), y(1) {}
	Rational(int x, int y);
	virtual ~Rational() = default;
	Rational(const Rational& other) :Integer(other.x), y(other.y) {}
	void Display() const;
};

Rational::Rational(int x, int y) :Integer(x), y(y) {
	if (this->y == 0) throw invalid_argument("错误！分母不能为零！");
	if (this->y < 0) {
		this->x = -this->x;
		this->y = -this->y;
	}
	int g = gcd(x, y);
	this->x /= g;
	this->y /= g;
}

void Rational::Display() const {
	if (x == 0) cout << 0;
	else cout << x << "/" << y;
}

class Complex :public Integer {
private:
	int y;
public:
	Complex() :Integer(0), y(0) {}
	Complex(int  x, int y) :Integer(x), y(y) {}
	void Display() const;
	virtual ~Complex() = default;
};

void Complex::Display() const {
	if (x == 0 && y == 0) cout << 0;
	else if (x != 0 && y == 0) cout << x;
	else if (x == 0 && y != 0) {
		if (abs(y) == 1) cout << (y > 0 ? "i" : "-i");
	}
	else {
		cout << x;
		if (y > 0) cout << "+";
		cout << y << "i";
	}
}

int main() {
	Rational x(9, 19);
	Complex z(3, -8);
	x.Display();
	z.Display();
	return 0;
}