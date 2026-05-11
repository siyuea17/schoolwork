/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw15_01
* Description: 使用输入输出流和随机数来生成随机数矩阵并读取
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/4/9 12:53:28
*/

// hw15_01.cpp
#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

using namespace std;

int main() {
    // 生成随机数
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> disInt(0, 1e9);

    // 生成随机数矩阵并填写
    double A[6][6] = { 0 };
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            A[i][j] = double(disInt(gen)) / 1e9;
        }
    }

    // 写入文本文件fout01.txt
    ofstream fout_txt;
    fout_txt.open("fout01.txt");
    if (!fout_txt.is_open()) {
        cerr << "fout01.txt 文件打开失败！" << endl;
        return 1;
    }
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            fout_txt << fixed << setprecision(9) << A[i][j];
            if (j < 5) fout_txt << " ";
        }
        fout_txt << "\n";
    }
    fout_txt.close();

    // 写入二进制文件fout01.dat
    ofstream fout_bin;
    fout_bin.open("fout01.dat", ios::binary);
    if (!fout_bin.is_open()) {
        cerr << "fout01.dat 文件打开失败！" << endl;
        return 1;
    }
    fout_bin.write(reinterpret_cast<const char*>(A), sizeof(A));
    fout_bin.close();

    // 读取二进制文件的前12个数据构成2x6矩阵B
    double B[2][6] = { 0 };
    ifstream fin_bin;
    fin_bin.open("fout01.dat", ios::binary);
    if (!fin_bin.is_open()) {
        cerr << "fout01.dat 文件打开失败，无法读取！" << endl;
        return 1;
    }
    fin_bin.read(reinterpret_cast<char*>(B), sizeof(B));
    fin_bin.close();

    // 输出矩阵B
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 6; ++j) {
            cout << fixed << setprecision(9) << B[i][j];
            if (j < 5) cout << " ";
        }
        cout << endl;
    }

    return 0;
}