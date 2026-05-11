/*
* Copyright (c) 2026,Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename:hw11_02.cpp
* Description:员工类
*
* Version:1.0
* Author:王新疆
* Date:2026年3月17日17点42分
*/

#include <iostream>
#include <string>
#include <iomanip>
#include <utility>

using namespace std;

class Employee {
public:
	Employee()
		:name(""), addr(""), city(""), zip("") {}

	Employee(string name0, string addr0, string city0, string zip0)
		:name(name0), addr(addr0), city(city0), zip(zip0) {}

	~Employee() {}

	Employee(const Employee& other)
		:name(other.name), addr(other.addr), city(other.city), zip(other.zip) {}

	Employee(Employee&& other) noexcept
		:name(std::move(other.name)),
		addr(std::move(other.addr)),
		city(std::move(other.city)),
		zip(std::move(other.zip)) {}

	Employee& operator=(const Employee& other) {
		if (this != &other) {
			name = other.name;
			addr = other.addr;
			city = other.city;
			zip = other.zip;
		}
		return *this;
	}

	Employee& operator=(Employee&& other) noexcept {
		if (this != &other) {
			name = move(other.name);
			addr = move(other.addr);
			city = move(other.city);
			zip = move(other.zip);
		}
		return *this;
	}

	void ChangeName(const string& name) {
		Employee::name = name;
	}

	void Display() const {
		cout << "这个员工的信息如下：" << endl
			<< "姓名：" << setw(10) << name << endl
			<< "地址：" << setw(10) << addr << endl
			<< "城市：" << setw(10) << city << endl
			<< "邮编：" << setw(10) << zip << endl;
	}
protected:
	string name, addr, city, zip;
};

int main() {
	Employee myself("王新疆", "保康县", "襄阳市", "441011");
	myself.Display();
}