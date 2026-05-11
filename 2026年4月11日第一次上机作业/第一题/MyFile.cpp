/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: 01
* Description: 
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/4/11 14:40:12
*/

#include "MyFile.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

void fireCampChain::txt1_to_txt2(const std::string& str1, const std::string& str2)
{
	fireCampChain fin;
	fin.open(str1);
	if (!fin.is_open()) {
		std::cerr << "文件a1打开失败";
		return;
	}

	//跳过标题行
	std::string line;
	std::getline(fin, line);

	std::vector<person> p;
	std::string name;
	int n, w, a;
	while (fin >> name >> n >> w >> a) {
		p.push_back({ name, n, w, a });
	}

	//按照职称编号、工龄和年龄排序
	std::sort(p.begin(), p.end(),
		[](const person& a, const person& b) {
			if (a.number != b.number) {
				return a.number < b.number;
			}
			if (a.work_years != b.work_years) {
				return a.work_years < b.work_years;
			}
			return a.age < b.age;
		});

	//开始写入
	fireCampChain fout;
	fout.open(str2);
	if (!fout.is_open()) {
		std::cerr << "文件a2打开失败";
		return;
	}
	
	fout << line << std::endl;
	for (person i : p) {
		fout << std::left << std::setw(7) << i.name << std::right << std::setw(8) << i.number << std::setw(8) << i.work_years << std::setw(8) << i.age << std::endl;
	}
	return;
}
