/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: toy.h
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026年4月18日08点07分
*/


#pragma once
#include <iostream>
class toy
{
protected:
	char song[500];
	char dance[500];
public:
	virtual void singing() { std::cout << "toy singing" << std::endl; };
	virtual void dancing() { std:: cout << "toy dancing" << std::endl; };
};