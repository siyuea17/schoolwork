/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw15_03
* Description:
*
* Version: 1.0
* Author: ÍõÐÂ½®
* Date: 2026/4/9 18:45:03
*/

#include "PolyInt.h"
#include <algorithm>
#include <cmath>

using namespace std;

PolyInt::PolyInt() : n(0), a(new int[1]) {
    a[0] = 0;
}

PolyInt::PolyInt(int n) : n(n), a(new int[n + 1]) {
    for (int i = 0; i <= n; ++i) a[i] = 0;
}

PolyInt::PolyInt(int n, int a[]) : n(n), a(new int[n + 1]) {
    for (int i = 0; i <= n; ++i) this->a[i] = a[i];
}

PolyInt::PolyInt(const PolyInt& other) : n(other.n), a(new int[n + 1]) {
    for (int i = 0; i <= n; ++i) a[i] = other.a[i];
}

PolyInt& PolyInt::operator=(const PolyInt& other) {
    if (this != &other) {
        delete[] a;
        n = other.n;
        a = new int[n + 1];
        for (int i = 0; i <= n; ++i) a[i] = other.a[i];
    }
    return *this;
}

double PolyInt::operator()(double x) const {
    double sum = 0;
    for (int i = 0; i <= n; ++i) {
        sum += a[i] * pow(x, i);
    }
    return sum;
}

void PolyInt::Display() const {
    cout << "[";
    for (int i = 0; i < n; ++i) cout << a[i] << ",";
    cout << a[n] << "]" << endl;
}

ostream& operator<<(ostream& os, const PolyInt& p) {
    os << "[";
    for (int i = 0; i < p.n; ++i) os << p.a[i] << ",";
    os << p.a[p.n] << "]";
    return os;
}

PolyInt operator+(const PolyInt& p, const PolyInt& q) {
    PolyInt res(max(p.n, q.n));
    for (int i = 0; i <= res.n; ++i) {
        int tp = (i <= p.n) ? p.a[i] : 0;
        int tq = (i <= q.n) ? q.a[i] : 0;
        res.a[i] = tp + tq;
    }
    return res;
}

PolyInt operator*(const PolyInt& p, const PolyInt& q) {
    PolyInt res(p.n + q.n);
    for (int i = 0; i <= p.n; ++i) {
        for (int j = 0; j <= q.n; ++j) {
            res.a[i + j] += p.a[i] * q.a[j];
        }
    }
    return res;
}

PolyInt::~PolyInt() {
    delete[] a;
}