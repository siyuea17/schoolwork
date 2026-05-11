/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw15_02
* Description:
*
* Version: 1.0
* Author: ÍõÐÂ½®
* Date: 2026/4/9 14:16:25
*/

#include <iostream>

using namespace std;

template<typename T>
class Complex {
protected:
    T real;
    T imag;
public:
    Complex() :real(0), imag(0) {}
    Complex(T x, T y) :real(x), imag(y) {}
    Complex(const Complex& other) :real(other.real), imag(other.imag) {}
    virtual ~Complex() = default;
    friend ostream& operator<<(ostream& os, const Complex<T>& x) {
        if (x.imag < 0) os << x.real << x.imag << "i";
        else if (x.imag > 0) os << x.real << "+" << x.imag << "i";
        else if (x.imag == 0) os << x.real;
        return os;
    }
    friend ostream& operator<<(ostream& os, const Complex<T>& c) {
        if (c.imag == 0) {
            os << c.real;
        }
        else if (c.imag > 0) {
            os << c.real << "+" << c.imag << "i";
        }
        else {
            os << c.real << c.imag << "i";
        }
        return os;
    }

    friend istream& operator>>(istream& is, Complex<T>& x) {
        is >> x.real >> x.imag;
        return is;
    }
};

int main() {

    return 0;
}