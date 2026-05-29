/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: _3779创建Animals类
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/5/15 20:33:27
*/

#include <string>
#include <iostream>

using namespace std;

class Animals {
public:
    // Write the constructor here.
    // Input params into brackets.
    Animals(string species, int id) :species(species), id(id) {
        // --- write your code here ---
        total++;
    }

    // Write the public instance variables and member functions here.
    static int total; // 用来计算 Animals 类的所有实例化对象的数量


    // For example: void printInfo()
    // --- write your code here ---
    void printInfo() {
        cout << "New animal -> Species: " << species << ", ID: " << id << endl;
    }

private:
    // Write the private instance variables and member functions here.
    // --- write your code here ---
    string species; // 用来表示动物的种类，如："Dog"、"Cat" 等
    int id; // 用来表示动物的编号

};

int Animals::total = 0;