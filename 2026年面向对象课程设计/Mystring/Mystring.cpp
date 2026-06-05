

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
    if(ms.str == nullptr) {
        str[0] = '\0';
        return;
    }
	strcpy(str, ms.str);
}

MyString::MyString(MyString&& ms) :len(ms.len), str(ms.str)
{
	ms.len = 0;
	ms.str = new char[1] {'\0'};
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
	while (is.get(ch) && isspace(static_cast<unsigned char>(ch))) {}
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
        buf[pos++] = ch;
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
	if(ch == str) return *this;
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
		len = newLen;
        delete[] str;
		str = newStr;
		return *this;
	}
}

MyString& MyString::operator=(const MyString& ms)
{
	if (this == &ms) return *this;
	
	char* newStr = new char[ms.len + 1];
	if (ms.str != nullptr) {
		strcpy(newStr, ms.str);
	} else {
		newStr[0] = '\0';
	}
	delete[] str;
	str = newStr;
	len = ms.len;
	return *this;
	
}

MyString MyString::operator+(const MyString& ms)
{
	MyString res;
	res.len = len + ms.len;
	res.str = new char[res.len + 1]{'\0'};
	strcpy(res.str, str);
	strcat(res.str, ms.str);
	return res;
}


//==========================重载比较运算符=================================
bool MyString::operator>(const MyString& ms)
{
	return strcmp(str, ms.str) > 0;
}

bool MyString::operator<(const MyString& ms)
{
	return strcmp(str, ms.str) < 0;
}

bool MyString::operator==(const MyString& ms)
{
	return strcmp(str, ms.str) == 0;
}

bool MyString::operator!=(const MyString& ms)
{
	return strcmp(str, ms.str) != 0;
}

//========================================================================
long MyString::GetLength() const
{
	return static_cast<long>(len);
}

MyString MyString::substr(int pos, int n)
{
	if (pos < 0 || pos >= len || n == 0) return MyString();
	int actualN = (pos + n > len) ? (len - pos) : n;
	char* buf = new char[actualN + 1];
	strncpy(buf, str + pos, actualN);
	buf[actualN] = '\0';
	MyString res(buf);
	delete[] buf;
	return res;
}

MyString MyString::substr(int pos)
{
	return this->substr(pos, len);
}

MyString MyString::replace(const MyString& ms1, const MyString& ms2)
{

	return MyString();
}

int MyString::find(const MyString& ms)
{
	if (len == 0) return -1;
	char* found = strstr(str, ms.str);
	if (found == nullptr) return -1;
	return found - str;
}

bool MyString::equalsIgnoreCase(const MyString& ms)
{
	if (len != ms.len) return false;
	for (int i = 0; i < len; ++i)
	{
		if (tolower(static_cast<unsigned char>(str[i])) !=
			tolower(static_cast<unsigned char>(ms.str[i])))
			return false;
	}
	return true;
}

bool MyString::contains(const MyString& ms)
{
	return find(ms) != -1;
}

bool MyString::startsWith(const MyString& ms)
{
	if (len < ms.len) return false;
	return strncmp(str, ms.str, ms.len) == 0;
}

bool MyString::endsWith(const MyString& ms)
{
	if (len < ms.len) return false;
	return strcmp(str + len - ms.len, ms.str) == 0;
}

bool MyString::Load(const char* fileName)
{
	try {
		std::ifstream fin(fileName);
		fin.seekg(0, std::ios::end);
		long fileSize = static_cast<long>(fin.tellg());
		fin.seekg(0, std::ios::beg);

		char* buf = new char[fileSize + 1];
		fin.read(buf, fileSize);
		buf[fin.gcount()] = '\0';
		fin.close();
		*this = buf;
		delete[] buf;

		return true;
	}
	catch(const std::exception& e) {
		std::cerr << "Error loading file: " << e.what() << std::endl;
		return false;
	}
}

bool MyString::Save(const char* fileName)
{
	try {
		std::ofstream fout(fileName);
		fout << str;
		return true;
	}
	catch(const std::exception& e) {
		std::cerr << "Error saving file: " << e.what() << std::endl;
		return false;
	}
}
