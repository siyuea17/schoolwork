/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw10_02
* Description: 用来把时间戳转成对应的年月日的一个类
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/17 19:23:15
*/

#include <iostream>
#include <ctime>

using namespace std;

class MyDate {
private:
	int year;
	int month;
	int day;
public:
	MyDate(unsigned long second = time(0));
	MyDate(int y, int m, int d);
	void showDay() const;
};

//判断闰年
bool isLeapYear(int y) {
	return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

//一个月的天数
int daysInMonth(int y, int m) {
	if (m == 2) {
		return isLeapYear(y) ? 29 : 28;
	}
	if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
	return 31;
}

MyDate::MyDate(unsigned long second) {
	const unsigned long second_per_day = 86400;
	unsigned long days = second / second_per_day;
	
	//计算总共有多少天，从1970年1月1日开始一直减去一年的天数，直到不够一年
	year = 1970;
	while (true) {
		int daysInYear = isLeapYear(year) ? 366 : 365;
		if (days >= daysInYear) {
			days -= daysInYear;
			year++;
		}
		else {
			break;
		}
	}

	month = 1;
	while (true) {
		int daysInMouth = daysInMonth(year, month);
		if (days >= daysInMouth) {
			days -= daysInMouth;
			month++;
		}
		else {
			break;
		}
	}

	day = days + 1;//日期要加一天
}

MyDate::MyDate(int y, int m, int d) : year(y), month(m), day(d) {}

void MyDate::showDay() const {
	cout << "当前时间: " << year << "-" << month << "-" << day << endl;
}

int main() {
	MyDate d1;
	MyDate d2(3456201512);
	d1.showDay();
	cout << "3456201512秒对应的日期是: ";
	d2.showDay();
	return 0;
}
