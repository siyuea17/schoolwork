#pragma once

#include <iostream>

using std::ostream;
using std::istream;

class MyString
{
public:
	// 无参构造函数
	MyString();

	// 有参构造函数
	MyString(const char* ch);

	// 拷贝构造函数，注意用深拷贝
	MyString(const MyString& ms);

	// 移动构造函数 
	MyString(MyString&& ms);

	// 移动赋值运算符重载函数 
	MyString& operator=(MyString&& ms);

	// 析构函数，释放内存
	virtual ~MyString();

	// 重载 <<
	friend ostream& operator << (ostream& os, const MyString& ms);

	// 重载 >>
	friend istream& operator >> (istream& is, MyString& ms);

	// 重载 [] 
	char& operator[] (int index);

	// 重载 =，注意用深拷贝
	MyString& operator= (const char* ch);

	// 重载赋值运算符
	MyString& operator= (const MyString& ms);

	// 重载 +，串连接操作
	MyString  operator+ (const MyString& ms);

	// 返回字符串内容的长度（以字节为单位）
	long GetLength() const;
	//重载>，字符串比较操作
	bool operator > (const MyString& ms) const;

	// 重载<，字符串比较操作
	bool operator < (const MyString& ms) const;

	// 重载==，字符串比较操作
	bool operator == (const MyString& ms) const;

	// 重载!=，字符串比较操作
	bool operator != (const MyString& ms) const;

	// 截取从 pos 开始（包括 0）的 n 个字符子串，并返回
	MyString substr(int pos, int n) const;

	// 截取从 pos 开始（包括 0）到末尾的所有字符子串，并返回
	MyString substr(int pos) const;

	// 用ms2替换ms1
	MyString replace(const MyString& ms1, const MyString& ms2) const;

	// 查找并返回字符串中第一次出现ms子串位置（包括0）
	int find(const MyString& ms) const;

	// 忽略大小写地比较两个字符串的内容（ms串）是否一模一样
	bool equalsIgnoreCase(const MyString& ms) const;

	// 判断一个字符串里面是否包含指定的内容（ms串）
	bool contains(const MyString& ms) const;

	// 判断一个字符串是否以指定的内容（ms串）开头
	bool startsWith(const MyString& ms) const;

	// 判断一个字符串是否以指定的内容（ms串）结尾
	bool endsWith(const MyString& ms) const;

	// 从文本文件读取数据到字符串对象中
	bool Load(const char* fileName);

	// 将字符串的内容保存到文本文件中
	bool Save(const char* fileName);

	// 正则表达式
	MyString regexReplace(const MyString& pattern, const MyString& replacement) const;


private:

	// 长度
	int len;

	// char 类型的指针 
	char* str;

};
