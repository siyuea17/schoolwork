/*
* Copyright (c) 2025,Wuhan xxxxx Co.,Ltd
* All rights reserved.
* 
* Filename:
* Description:
* 
* Version:1.1
* Author:王新疆
* Date:2026年3月12日13点26分
* 修复了复数的显示问题
*
* Version:1.0
* Author:王新疆
* Date:2026年3月10日13点42分
*/

#include <iostream>
#include <cmath>

using namespace std;

class Complex {
private:
	double x;
	double y;
public:
	Complex();
	Complex(double x, double y);
	double getx() const;
	double gety() const;
	void Display() const;
	double Abs() const;
	Complex Minus(Complex&) const;
	Complex multiply(Complex&) const;
};

Complex::Complex():x(0),y(0){}
Complex::Complex(double x, double y) :x(x), y(y) {}

double Complex::getx() const {
	return x;
}

double Complex::gety() const {
	return y;
}

void Complex::Display() const {
	if (x == 0 && y == 0) {
		cout << 0;
	}
	else if (x == 0) {
		if (abs(y) == 1) {
			cout << (y > 0 ? "+" : "-") << "i";
		}
		else {
			cout << y << "i";
		}
	}
	else if (y == 0) {
		cout << x;
	}
	else {
		cout << x;
		if (y > 0) cout << "+";
		cout << y << "i";
	}
	cout << endl;
}
double Complex::Abs() const {
	return sqrt(x * x + y * y);
}
Complex Complex::Minus(Complex& a) const {
	Complex res(this->getx()-a.getx(),this->gety() - a.gety());
	return res;
}

Complex Complex::multiply(Complex& a) const {
	Complex res(this->getx() * a.getx() - this->gety() * a.gety(), this->gety() * a.getx() + this->getx() * a.gety());
	return res;
}

int main() {
	Complex z1(1.4, -2.3);
	Complex z2(-3.5, +2.7);

	cout << z1.Abs() << endl;

	z1.Minus(z2).Display();
	z1.multiply(z2).Display();

	return 0;
}
