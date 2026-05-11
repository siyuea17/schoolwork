/*
* Copyright (c) 2026,Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename:hw11_03
* Description:字符易位破译，使用string类实现
*
* Version:1.0
* Author:王新疆
* Date:2026年3月17日17点41分
*/

#include <iostream>
#include <string>

using namespace std;

inline void swap(char& ch1, char& ch2) {
	char temp = ch1;
	ch1 = ch2;
	ch2 = temp;
}

bool isAnagram(const string& s1, const string& s2) {
	if (s1.empty() && s2.empty()) {
		return true;
	}

	if (s1.length() != s2.length()) {
		return false;
	}

	string str1 = s1;
	string str2 = s2;

	bool swapped = true;
	int n = str1.length();
	int i, j;
	for (i = 0; i < n - 1; i++) {
		swapped = false;
		for (j = 0; j < n - i - 1; j++) {
			if (str1[j] > str1[j + 1]) {
				swap(str1[j], str1[j + 1]);
				swapped = true;
			}
		}
		if (!swapped) break;
	}

	for (i = 0; i < n - 1; i++) {
		swapped = false;
		for (j = 0; j < n - i - 1; j++) {
			if (str2[j] > str2[j + 1]) {
				swap(str2[j], str2[j + 1]);
				swapped = true;
			}
		}
		if (!swapped) break;
	}

	for (i = 0; i < n; i++) {
		if (str1[i] != str2[i]) {
			return false;
		}
	}
	return true;
}

int main() {
	string s1, s2;
	s1 = "sssdddccvrf";
	s2 = "dddccvfsvss";
	cout << isAnagram(s1, s2);
}