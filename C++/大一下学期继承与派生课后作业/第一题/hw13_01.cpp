/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw13_01
* Description: 分数类
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/19 16:21:04
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
	int a;
public:
	Integer() :a(0) {}
	Integer(int x) :a(x) {}
	~Integer() = default;
	Integer(const Integer& other) { a = other.a; }
};

class Fraction :public Integer {
protected:
	int b;
public:
	Fraction() :Integer(0), b(1) {}
	Fraction(int a, int b);
	~Fraction() = default;
	Fraction(const Fraction& other) { a = other.a; b = other.b; }
	void display() const;
	friend Fraction operator+(const Fraction&, const Fraction&);
	friend int gcd(int, int);

};
Fraction::Fraction(int a, int b) :Integer(a), b(b) {
	if (b == 0) {
		throw invalid_argument("分母不能为0！");
	}
	//分母改成正的
	if (b < 0) {
		this->b = -this->b;
		this->a = -this->a;
	}
}

void Fraction::display() const {
	cout << a << "/" << b;
}

Fraction operator+(const Fraction& x, const Fraction& y) {
	int new_a = x.a * y.b + x.b * y.a;
	int new_b = x.b * y.b;
	int g = gcd(new_a, new_b);
	new_a /= g;
	new_b /= g;
	return Fraction(new_a, new_b);
}

int main() {
	Fraction x(2, 3), y(-1, 6);
	Fraction z = x + y;
	z.display();
	return 0;
}


