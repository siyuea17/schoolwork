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

#pragma once
#include <fstream>

struct person {
		std::string name;
		int number, work_years, age;
};

class fireCampChain:public std::fstream, public person
{
protected:
	std::fstream file;
public:
	void txt1_to_txt2(const std::string& str1, const std::string& str2);
	virtual ~fireCampChain();
};