/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw15_03
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/4/9 18:45:03
*/

#include <iostream>
#include "PolyInt.h"

using namespace std;

int main() {
    int arr1[] = { 1, 2, 0, 4 };
    PolyInt p(3, arr1);

    int arr2[] = { 2, 0, -4, -3, 0, 1 };
    PolyInt q(5, arr2);

    PolyInt s = p * q;

    cout << "p(2)的值为：" << p(2) << endl;
    cout << "s(2)的值为：" << s(2) << endl;

    s.Display();
    return 0;
}