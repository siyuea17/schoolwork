/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw10_01
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/17 19:08:24
*/

#include <iostream>

using namespace std;

class Score {
public:
    Score() :math(0), eng(0) {}
    Score(int a, int b) :math(a), eng(b) {}
    ~Score(){}
    void show() const;
private:
    int math, eng;
};

void Score::show() const {
    cout << "数学成绩是：" << math << " ，英语成绩是：" << eng << endl;
}

class Student {
private:
    int stuid;
    Score mark;
public:
    Student() :mark(),stuid(0) {}
    Student(int stu,int a,int b) :stuid(stu), mark(a,b){}
    ~Student() {}
    void stushow();
};

void Student::stushow() {
    cout << "学生的学号为：" << stuid << endl;
    mark.show();
}

int main() {
    Student Anna(2017007, 98, 85);
    Anna.stushow();
    return 0;
}