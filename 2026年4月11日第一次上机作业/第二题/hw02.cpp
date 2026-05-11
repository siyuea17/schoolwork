/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw02
* Description:
*
* Version: 1.0
* Author: ÍõÐÂ½®
* Date: 2026/4/11 15:09:17
*/

#include <iostream>
#include "fireCampChain.h"

int main() {
    std::string firemap = "c1.txt", outmap = "c2.txt";

    int x, y;
    std::cin >> x >> y;

    fireCampChain c1;
    c1.fire(firemap, outmap, x, y);
    return 0;
}