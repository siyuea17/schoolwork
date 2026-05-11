/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw14_01
* Description: 
*
* Version: 1.0
* Author: ÍõÐÂ½®
* Date: 2026/3/26 20:09:28
*/

#include <iostream>
#include <cmath>

using namespace std;

class Real {
protected:
	float x;
public:
	Real() :x(0) {}
	Real(float x) :x(x) {}
	virtual ~Real() = default;
	virtual float Dist(float r);
	Real(const Real& r) :x(r.x) {}
};

float Real::Dist(float r) {
	return abs(this->x - r);
}

class Complex :public Real {
private:
	float y;
public:
	Complex() :Real(0), y(0) {}
	Complex(float x, float y) :Real(x), y(y) {}
	Complex(const Complex& other) :Real(other.x), y(other.y) {}
	virtual float Dist(float r);
	virtual ~Complex() = default;
};

float Complex::Dist(float r) {
	return sqrt((x - r) * (x - r) + y * y);
}

int main() {
	Real r1(1.2);
	Complex z1(3.14, -2.78);
	Real* ptr1 = &r1, * ptr2 = &z1;
	cout << ptr1->Dist(2.24) << endl;
	cout << ptr2->Dist(2.24) << endl;
	return 0;
}

