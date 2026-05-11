/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: 01
* Description:
*
* Version: 1.0
* Author: ÍõÐÂ½®
* Date: 2026/4/11 14:40:12
*/

#include "MyFile.h"

int main() {
    std::string str1, str2;
    str1 = "a1.txt";
    str2 = "a2.txt";

    fireCampChain f;
    f.txt1_to_txt2(str1, str2);
    return 0;
}