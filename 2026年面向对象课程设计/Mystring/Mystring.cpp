#include "MyString.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <cstring>   // strlen, strcpy, strncpy, strstr, strcmp
#include <cctype>    // tolower
#include <stdexcept> // std::out_of_range


// ====================构造函数和析构函数=========================

MyString::MyString() :len(0), str(new char[1] {'\0'})
{

}

MyString::MyString(const char* ch) :len(0), str(new char[1] {'\0'})
{
	if (ch != nullptr) {
		len = strlen(ch);
		str = new char[len + 1];
		strcpy(str, ch);
	}
}

MyString::MyString(const MyString& ms) :len(ms.len), str(new char[len + 1])
{
	strcpy(str, ms.str);
}

MyString::MyString(MyString&& ms) :len(ms.len), str(ms.str)
{
	delete[] str;
	ms.len = 0;
	str = new char[1] {'\0'};
}

MyString& MyString::operator=(MyString&& ms)
{
	if (this == &ms) return *this;
	delete[] str;
	str = ms.str;
	len = ms.len;
	ms.str = new char[1];
	ms.str[0] = '\0';
	ms.len = 0;
	return *this;
}

MyString::~MyString()
{
	delete[] str;
}

//====================================================================

//===========================运算符重载================================

ostream& operator<<(ostream& os, MyString& ms)
{
	os << (ms.str ? ms.str : "");
	return os;
}

istream& operator>>(istream& is, MyString& ms)
{
	char ch;
	while(is.get(ch) && isspace(static_cast<unsigned char>(ch))){}
	if (!is) return is;

	int cap = 64;
	int pos = 0;
	char* buf = new char[cap];
	buf[pos++] = ch;

	while (is.get(ch) && !isspace(static_cast<unsigned char>(ch))) {
		if (pos >= cap - 1) {
			cap *= 2;
			char* newBuf = new char[cap];
			memcpy(newBuf, buf, pos);
			delete[] buf;
			buf = newBuf;
		}
	}
	buf[pos++] = '\0';
	ms = buf;
	delete[] buf;
	return is;
}

char& MyString::operator[](int index)
{
	if (index < 0 || index >= len)
		throw std::out_of_range("MyString::operator[] index out of range");
	return str[index];
}

MyString& MyString::operator=(const char* ch)
{
	if (ch == nullptr) {
		delete[] str;
		str = new char[1] {'\0'};
		len = 0;
		return *this;
	}
	else {
		int newLen = strlen(ch);
		char* newStr = new char[newLen + 1];
		strcpy(newStr, ch);


	}
}

MyString& MyString::operator=(MyString& ms)
{
	// TODO: 在此处插入 return 语句
}

MyString MyString::operator+(MyString& ms)
{
	return MyString();
}



bool MyString::operator>(const MyString& ms)
{
	return false;
}

bool MyString::operator<(const MyString& ms)
{
	return false;
}

bool MyString::operator==(const MyString& ms)
{
	return false;
}

bool MyString::operator!=(const MyString& ms)
{
	return false;
}

//=====================================
long MyString::GetLength()
{
	return 0;
}

MyString MyString::substr(int pos, int n)
{
	return MyString();
}

MyString MyString::substr(int pos)
{
	return MyString();
}

MyString MyString::replace(const MyString& ms1, const MyString& ms2)
{
	return MyString();
}

int MyString::find(const MyString& ms)
{
	return 0;
}

bool MyString::equalsIgnoreCase(const MyString& ms)
{
	return false;
}

bool MyString::contains(const MyString& ms)
{
	return false;
}

bool MyString::startsWith(const MyString& ms)
{
	return false;
}

bool MyString::endsWith(const MyString& ms)
{
	return false;
}

bool MyString::Load(const char* fileName)
{
	return false;
}

bool MyString::Save(const char* fileName)
{
	return false;
}



