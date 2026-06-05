

#include "Mystring.h"

#include <iostream>

using namespace std;

int main()
{
	MyString s1("Hello");
	MyString s2 = s1; // 复制构造
	MyString s3; 
	s3 = s1; // 复制赋值
	MyString s4 = std::move(s1); // 移动构造
	MyString s5;
	s5 = std::move(s2); // 移动赋值
	cout << "s1: " << s1 << endl; // s1 已被移动，应该是空字符串
	cout << "s2: " << s2 << endl; // s2 已被移动，应该是空字符串
	cout << "s3: " << s3 << endl; // s3 应该是 "Hello"
	cout << "s4: " << s4 << endl; // s4 应该是 "Hello"
	cout << "s5: " << s5 << endl; // s5 应该是 "Hello"
	return 0;
}