/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw12_03
* Description:实数类，重载关系运算符<、>、==
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/24 20:25:22
*/

#include <iostream>

using namespace std;

class Rational {
private:
	int numer, denom;
public:
	Rational() :numer(0), denom(1) {}
	Rational(int a) :numer(a), denom(1) {}
	Rational(int a, int b);
	Rational(const Rational& other) :numer(other.numer), denom(other.denom) {}
	virtual ~Rational() = default;

	bool operator<(const Rational& other) const;
	bool operator>(const Rational& other) const;
	bool operator==(const Rational& other) const;
};

Rational::Rational(int a,int b):numer(a), denom(b) {
	if (this->numer == 0) throw invalid_argument("错误！分母不能为零！");
	if (this->denom < 0) {
		this->numer = -this->numer;
		this->denom = -this->denom;
	}
}

bool Rational::operator<(const Rational& other) const {
	return numer * other.denom < other.numer * denom;
}

bool Rational::operator>(const Rational& other) const {
	return numer * other.denom > other.numer * denom;
}

bool Rational::operator==(const Rational& other) const {
	return numer * other.denom == other.numer * denom;
}

int main() {
	Rational a(4, 5), b(2, 3);
	cout << "a>b?" << (a > b ? "true" : "false") << endl;
	cout << "a==b?" << (a == b ? "true" : "false") << endl;
	cout << "a<b?" << (a < b ? "true" : "false") << endl;
	return 0;
}