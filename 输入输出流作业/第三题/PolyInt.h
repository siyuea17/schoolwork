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

#pragma once

#include <iostream>

class PolyInt {
public:
    PolyInt();
    PolyInt(int n);
    PolyInt(int n, int a[]);
    PolyInt(const PolyInt& other);
    PolyInt& operator=(const PolyInt& other);
    double operator()(double x) const;
    void Display() const;
    friend std::ostream& operator<<(std::ostream& os, const PolyInt& p);
    friend PolyInt operator+(const PolyInt& p, const PolyInt& q);
    friend PolyInt operator*(const PolyInt& p, const PolyInt& q);
    virtual ~PolyInt();

private:
    int n;
    int* a;
};
