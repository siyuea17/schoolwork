/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: beartoy.h
* Description:
*
* Version: 1.0
* Author: 王新疆
* Date: 2026年4月18日08点14分
*/

#pragma once
#include "toy.h"
#include <string>
class Beartoy : public toy
{
public:
	Beartoy();
	Beartoy(const std::string& str1, const std::string& str2);

	void singing() override;
	void dancing() override;
};
