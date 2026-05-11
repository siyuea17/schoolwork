# FractionDll - 分数运算动态库

## 简介
提供分数加减乘除及化简功能的动态链接库，支持 C++ 流输出。

## 编译环境
- Visual Studio 2022

## 使用示例
```c++
#include "Fraction.h"
#include <iostream>

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
```