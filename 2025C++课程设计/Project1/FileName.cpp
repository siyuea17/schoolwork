#include <iostream>

using namespace std;

class String {
	char* string;
	int size_of_string;

public:
	String();
	String(const char* str);
	String(const String& other);
	~String();

	String& operator=(const char* arr);
	String& operator=(const String& s);

	int size(const char* arr) const;//获取一个已有的字符串的元素个数

	String operator+(const char* arr);
	String operator+(const String& s);

	friend ostream& operator<<(ostream& os, const String& s);

	char& operator[](int index);
	const char& operator[](int index) const;

	String& operator+=(const String& s);

	static int strcmp(const String& s1, const String& s2);
	String& strcpy(const String& s);
	String& strcat(const String& s);
	
	bool empty() const {
		return size_of_string == 0;
	}
	
	int index(const char* substr);
	int lastIndex(const char* str);
	String toUpperCase() const;
	String replace(const char* oldStr, const char* newStr) const;
};

String::String() :string(nullptr), size_of_string(0) {
	string = new char[1];
	string[0] = '\0';
}

String::String(const char* str) :string(nullptr), size_of_string(0) {
	if (str) {
		size_of_string = size(str);
		string = new char[size_of_string + 1];
		for (int i = 0; i < size_of_string; i++) {
			string[i] = str[i];
		}
		string[size_of_string] = '\0';
	}
	else {
		string = new char[1];
		string[0] = '\0';
		size_of_string = 0;
	}
}

String::String(const String& other) :string(nullptr), size_of_string(other.size_of_string) {
	if (other.string) {
		string = new char[size_of_string + 1];
		for (int i = 0; i <= size_of_string; i++) {
			string[i] = other.string[i];
		}
	}
	else {
		string = new char[1];
		string[0] = '\0';
	}
}

String::~String() {
	if (string != nullptr) {
		delete[] string;
	}
}

int String::size(const char* arr) const{
	int n(0);
	if (arr == nullptr) return 0;
	while (arr[n] != '\0') {
		n++;
	}
	return n;
}

String& String::operator=(const char* arr) {
	if (string != nullptr) {
		delete[] string;
	}
	size_of_string = size(arr);
	string = new char[size_of_string + 1];
	for (int i = 0; i <= size_of_string; i++) {
		string[i] = arr[i];
	}
	return *this;
}

String& String::operator=(const String& s) {
	if (this != &s) {
		if (string) {
			delete[] string;
		}
		size_of_string = s.size_of_string;
		if (s.string) {
			string = new char[size_of_string + 1];
			for (int i = 0; i <= size_of_string; i++) {
				string[i] = s.string[i];
			}
		}
		else {
			string = new char[1];
			string[0] = '\0';
		}
	}
	return *this;
}

String String::operator+(const String& s) {
	String result;
	result.size_of_string = size_of_string + s.size_of_string;
	result.string = new char[result.size_of_string + 1];

	for (int i = 0; i < size_of_string; i++) {
		result.string[i] = string[i];
	}

	for (int i = 0; i < s.size_of_string; i++) {
		result.string[size_of_string + i] = s.string[i];
	}

	result.string[result.size_of_string] = '\0';
	return result;
}

String String::operator+(const char* arr) {
	int arr_len = size(arr);

	String result;
	result.size_of_string = size_of_string + arr_len;
	result.string = new char[result.size_of_string + 1];

	for (int i = 0; i < size_of_string; i++) {
		result.string[i] = string[i];
	}

	for (int i = 0; i < arr_len; i++) {
		result.string[size_of_string + i] = arr[i];
	}

	result.string[result.size_of_string] = '\0';
	return result;
}

ostream& operator<<(ostream& os, const String& s) {
	if (s.string) {
		os << s.string;
	}
	return os;
}

char& String::operator[](int index) {
	if (index < 0 || index >= size_of_string) {
		static char error = '\0';
		return error;
	}
	return string[index];
}

const char& String::operator[](int index) const {
	if (index < 0 || index >= size_of_string) {
		static char error = '\0';
		return error;
	}
	return string[index];
}

String& String::operator+=(const String& s) {
	if (s.size_of_string == 0) {
		return *this;
	}

	int new_size = size_of_string + s.size_of_string;
	char* new_string = new char[new_size + 1];

	for (int i = 0; i < size_of_string; i++) {
		new_string[i] = string[i];
	}

	for (int i = 0; i < s.size_of_string; i++) {
		new_string[size_of_string + i] = s.string[i];
	}

	new_string[new_size] = '\0';

	delete[] string;
	string = new_string;
	size_of_string = new_size;

	return *this;
}

int String::strcmp(const String& s1, const String& s2) {
	//有空的直接返回
	if (s1.string == nullptr && s2.string == nullptr) return 0;
	if (s1.string == nullptr) return -1;
	if (s2.string == nullptr) return 1;

	int i = 0;
	while (s1.string[i] != '\0' && s2.string[i] != '\0') {
		if (s1.string[i] > s2.string[i]) {
			return 1;
		}
		else if (s1.string[i] < s2.string[i]) {
			return -1;
		}
		i++;
	}
	if (s1.string[i] == '\0' && s2.string[i] != '\0') return -1;
	if (s1.string[i] != '\0' && s2.string[i] == '\0') return 1;

	return 0;
}

String& String::strcpy(const String& s) {
	*this = s;
	return *this;
}

String& String::strcat(const String& s) {
	*this += s;
	return *this;
}

int String::index(const char* str) {
	if (size(str) == 0) {
		return -1;
	}
	if (size(str) > size_of_string) return -1;

	for (int i = 0; i <= size_of_string - size(str); i++) {
		bool found = true;
		for (int j = 0; j < size(str); j++) {
			if (string[i + j] != str[j]) {
				found = false;
				break;
			}
		}
		if (found) {
			return i;
		}
	}
	return -1;
}

int String::lastIndex(const char* str) {
	int sub_len = size(str);
	if (sub_len == 0 || sub_len > size_of_string) return -1;
	for (int i = size_of_string - sub_len; i >= 0; i--) {
		bool found = true;
		for (int j = 0; j < sub_len; j++) {
			if (string[i + j] != str[j]) {
				found = false;
				break;
			}
		}
		if (found) {
			return i;
		}
	}
	return -1;
}

String String::toUpperCase() const {
	String result = *this;
	for (int i = 0; i < size_of_string; i++) {
		if (result[i] >= 'a' && result[i] <= 'z') {
			result[i] = result[i] + 'A' - 'a';
		}
	}
	return result;
}

String String::replace(const char* oldStr, const char* newStr) const{
	int old_len = size(oldStr);
	int new_len = size(newStr);

	if (old_len == 0) return *this;

	int count = 0;
	for (int i = 0; i <= size_of_string - old_len; i++) {
		bool match = true;
		for (int j = 0; j < old_len; j++) {
			if (string[i + j] != oldStr[j]) {
				match = false;
				break;
			}
		}
		if (match) {
			count++;
			i += old_len - 1;//跳过已匹配部分
		}
	}
	if (count == 0) return *this;

	int new_size = size_of_string + count * (new_len - old_len);//计算新字符串的长度
	char* new_data = new char[new_size + 1];
	int pos = 0;
	int i = 0;

	while (i < size_of_string) {
		bool match = false;
		if (i <= size_of_string - old_len) {
			match = true;
			for (int j = 0; j < old_len; j++) {
				if (string[i + j] != oldStr[j]) {
					match = false;
					break;
				}
			}
		}
		if (match) {
			for (int j = 0; j < new_len; j++) {
				new_data[pos++] = newStr[j];
			}
			i += old_len;
		}
		else {
			new_data[pos++] = string[i++];
		}
	}

	new_data[new_size] = '\0';
	String result(new_data);
	delete[] new_data;
	return result;
}

int main() {
	// 测试构造函数
	String str1 = "hello";      // 构造
	String str2;                // 默认构造
	String str3 = str1;         // 拷贝构造

	cout << "测试 empty() 函数：" << endl;
	cout << "str1.empty(): " << str1.empty() << endl;   // 0 (false)
	cout << "str2.empty(): " << str2.empty() << endl;   // 1 (true)

	cout << "\n测试 index() 函数：" << endl;
	cout << "str3.index(\"llo\"): " << str3.index("llo") << endl;  // 输出 2

	cout << "\n测试 lastIndex() 函数：" << endl;
	cout << "str3.lastIndex(\"llo\"): " << str3.lastIndex("llo") << endl;  // 输出 2

	// 测试有多个匹配的情况
	String str4 = "hello world hello";
	cout << "str4.lastIndex(\"hello\"): " << str4.lastIndex("hello") << endl;  // 输出 12

	cout << "\n测试 toUpperCase() 函数：" << endl;
	cout << "str1.toUpperCase(): " << str1.toUpperCase() << endl;  // 输出 "HELLO"

	cout << "\n测试 replace() 函数：" << endl;
	cout << "str1.replace(\"l\", \"O\"): " << str1.replace("l", "O") << endl;  // 输出 heOOo

	cout << "\n测试 strcmp() 函数：" << endl;
	String s1 = "abc";
	String s2 = "def";
	String s3 = "abc";
	cout << "String::strcmp(\"abc\", \"def\"): " << String::strcmp(s1, s2) << endl;  // -1
	cout << "String::strcmp(\"def\", \"abc\"): " << String::strcmp(s2, s1) << endl;  // 1
	cout << "String::strcmp(\"abc\", \"abc\"): " << String::strcmp(s1, s3) << endl;  // 0

	cout << "\n测试 strcpy() 和 strcat() 函数：" << endl;
	String s4 = "Hello";
	String s5 = " World";
	s4.strcpy(s5);
	cout << "s4.strcpy(s5): " << s4 << endl;  // 输出 " World"

	String s6 = "Hello";
	s6.strcat(s5);
	cout << "s6.strcat(s5): " << s6 << endl;  // 输出 "Hello World"

	cout << "\n测试运算符重载：" << endl;
	String s7 = "Hello";
	String s8 = " World";
	String s9 = s7 + s8;
	cout << "s7 + s8: " << s9 << endl;  // 输出 "Hello World"

	s7 += s8;
	cout << "s7 += s8: " << s7 << endl;  // 输出 "Hello World"

	cout << "\n测试下标运算符：" << endl;
	String s10 = "Hello";
	cout << "s10[0]: " << s10[0] << endl;  // 输出 'H'
	s10[0] = 'h';
	cout << "修改后 s10: " << s10 << endl;  // 输出 "hello"

	// 测试 const 版本的下标运算符
	const String s11 = "Hello";
	cout << "const s11[0]: " << s11[0] << endl;  // 输出 'H'
	// s11[0] = 'h';  // 错误！不能修改 const 对象

	return 0;
}
//int main() {
//	String str1 = "hello"; //构造
//	String str2;
//	String str3 = str1; //copy constructor
//	cout << str3.index("llo") << endl;// 输出 2,表示"llo"在 str3 中下标为 2 的位置第一次出现的。如果不存在，则返回 - 1;
//	cout << str3.lastIndex("llo") << endl;// 输出 7,表示"llo"在str3中下标为7的位置最后一次出现的。
//	cout << str1.toUpperCase() << endl;//输出 "HELLO"
//	cout << str1.replace("l", "O") << endl; //输出 heOOo 
//}