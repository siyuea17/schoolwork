/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw15_04
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/4/9 20:58:26
*/

// hw15_04.cpp
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>

using namespace std;

std::string GBKToUTF8(const std::string& gbkStr) {
    int len = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, &wstr[0], len);

    len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8Str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], len, NULL, NULL);

    return utf8Str;
}

int main() {
    // 设置控制台输出编码为UTF-8
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // 读取名单
    vector<string> namelist;
    ifstream fin("namelist.txt");
    if (!fin) {
        cerr << u8"文件 namelist.txt 打开失败！" << endl;
        return 1;
    }
    string name;
    while (fin >> name) {
        namelist.push_back(name);
    }
    fin.close();

    // 至少需要 1+2+3=6 人，且还有剩余
    int N = namelist.size();
    cout << u8"总参与人数：" << N << endl;
    if (N < 7) {  
        cerr << u8"人数过少，无法满足 N3>N2>N1 且总和小于 N" << endl;
        return 1;
    }

    // 随机生成奖项人数 N1, N2, N3（满足 N3 > N2 > N1 且 N1+N2+N3 < N）
    random_device rd;
    mt19937 gen(rd());

    // 计算 N1 的最大值，确保剩余人数足够 N2, N3 和至少 1 个未中奖
    int maxN1 = (N - 6) / 3;
    uniform_int_distribution<int> disN1(1, maxN1);
    int N1 = disN1(gen);

    // N2 的范围：[N1+1, (N - N1 - 3) / 2]
    int maxN2 = (N - N1 - 3) / 2;
    uniform_int_distribution<int> disN2(N1 + 1, maxN2);
    int N2 = disN2(gen);

    // N3 的范围：[N2+1, N - N1 - N2 - 1]
    int maxN3 = N - N1 - N2 - 1;
    uniform_int_distribution<int> disN3(N2 + 1, maxN3);
    int N3 = disN3(gen);

    
    cout << u8"一等奖 " << N1 << u8" 人，二等奖 " << N2 << u8" 人，三等奖 " << N3 << u8" 人" << endl;
    cout << u8"未中奖人数：" << N - N1 - N2 - N3 << endl;

    // 打乱池子
    shuffle(namelist.begin(), namelist.end(), gen);

    // 打乱整个名单
    shuffle(namelist.begin(), namelist.end(), gen);

    // 顺序取奖
    cout << u8"\n三等奖 (" << N3 << u8" 人)：" << endl;
    for (int i = 0; i < N3; ++i) cout << namelist[i] << endl;

    cout << u8"\n二等奖 (" << N2 << u8" 人)：" << endl;
    for (int i = N3; i < N3 + N2; ++i) cout << namelist[i] << endl;

    cout << u8"\n一等奖 (" << N1 << u8" 人)：" << endl;
    for (int i = N3 + N2; i < N3 + N2 + N1; ++i) cout << namelist[i] << endl;

    cout << u8"\n未中奖名单 (" << N - N1 - N2 - N3 << u8" 人)：" << endl;
    for (int i = N3 + N2 + N1; i < N; ++i) cout << namelist[i] << endl;

    return 0;
}