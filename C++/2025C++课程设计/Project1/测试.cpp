//int main() {
//	// 测试构造函数
//	String str1 = "hello";      // 构造
//	String str2;                // 默认构造
//	String str3 = str1;         // 拷贝构造
//
//	cout << "测试 empty() 函数：" << endl;
//	cout << "str1.empty(): " << str1.empty() << endl;   // 0 (false)
//	cout << "str2.empty(): " << str2.empty() << endl;   // 1 (true)
//
//	cout << "\n测试 index() 函数：" << endl;
//	cout << "str3.index(\"llo\"): " << str3.index("llo") << endl;  // 输出 2
//
//	cout << "\n测试 lastIndex() 函数：" << endl;
//	cout << "str3.lastIndex(\"llo\"): " << str3.lastIndex("llo") << endl;  // 输出 2
//
//	// 测试有多个匹配的情况
//	String str4 = "hello world hello";
//	cout << "str4.lastIndex(\"hello\"): " << str4.lastIndex("hello") << endl;  // 输出 12
//
//	cout << "\n测试 toUpperCase() 函数：" << endl;
//	cout << "str1.toUpperCase(): " << str1.toUpperCase() << endl;  // 输出 "HELLO"
//
//	cout << "\n测试 replace() 函数：" << endl;
//	cout << "str1.replace(\"l\", \"O\"): " << str1.replace("l", "O") << endl;  // 输出 heOOo
//
//	cout << "\n测试 strcmp() 函数：" << endl;
//	String s1 = "abc";
//	String s2 = "def";
//	String s3 = "abc";
//	cout << "String::strcmp(\"abc\", \"def\"): " << String::strcmp(s1, s2) << endl;  // -1
//	cout << "String::strcmp(\"def\", \"abc\"): " << String::strcmp(s2, s1) << endl;  // 1
//	cout << "String::strcmp(\"abc\", \"abc\"): " << String::strcmp(s1, s3) << endl;  // 0
//
//	cout << "\n测试 strcpy() 和 strcat() 函数：" << endl;
//	String s4 = "Hello";
//	String s5 = " World";
//	s4.strcpy(s5);
//	cout << "s4.strcpy(s5): " << s4 << endl;  // 输出 " World"
//
//	String s6 = "Hello";
//	s6.strcat(s5);
//	cout << "s6.strcat(s5): " << s6 << endl;  // 输出 "Hello World"
//
//	cout << "\n测试运算符重载：" << endl;
//	String s7 = "Hello";
//	String s8 = " World";
//	String s9 = s7 + s8;
//	cout << "s7 + s8: " << s9 << endl;  // 输出 "Hello World"
//
//	s7 += s8;
//	cout << "s7 += s8: " << s7 << endl;  // 输出 "Hello World"
//
//	cout << "\n测试下标运算符：" << endl;
//	String s10 = "Hello";
//	cout << "s10[0]: " << s10[0] << endl;  // 输出 'H'
//	s10[0] = 'h';
//	cout << "修改后 s10: " << s10 << endl;  // 输出 "hello"
//
//	// 测试 const 版本的下标运算符
//	const String s11 = "Hello";
//	cout << "const s11[0]: " << s11[0] << endl;  // 输出 'H'
//	// s11[0] = 'h';  // 错误！不能修改 const 对象
//
//	return 0;
//}