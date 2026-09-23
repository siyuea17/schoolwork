/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: Test
* Description:
*
* Version: 1.0
* Author: ÍõÐÂ½®
* Date: 2026/4/15 0:00:29
*/

#include <iostream>
#include "Fraction.h"

int main() {
    Fraction a(1, 2);   // 1/2
    Fraction b(1, 3);   // 1/3
    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "a + b = " << a + b << std::endl;
    std::cout << "a - b = " << a - b << std::endl;
    std::cout << "a * b = " << a * b << std::endl;
    std::cout << "a / b = " << a / b << std::endl;
    Fraction c;
    std::cin >> c; // input "1" "/" "2"
    std::cout << "The Fraction you enter is " << c << std::endl;
    return 0;
}