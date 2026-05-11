#include <iostream>
#include <string>
using namespace std;

string encryption(const string& input) {
	string result = "";
	for (char ch : input) {
		if (ch >= 'a' && ch <= 'y') {
			ch += 1;
		}
		else if (ch == 'z') {
			ch = 'a';
		}
		if (ch >= 'A' && ch <= 'Y') {
			ch += 1;
		}
		else if (ch == 'Z') {
			ch = 'A';
		}
		result += ch;
	}
	return result;
}

string decryption(const string& input) {
	string result = "";
	for (char ch : input) {
		if (ch >= 'b' && ch <= 'z') {
			ch -= 1;
		}
		else if (ch == 'a') {
			ch = 'z';
		}
		if (ch >= 'B' && ch <= 'Z') {
			ch -= 1;
		}
		else if (ch == 'A') {
			ch = 'Z';
		}
		result += ch;
	}
	return result;
}

int main() {
	string s = "abcdefghijklmnopqrstuvwxyz !!! ABCDEFGHIJKLMNOPQRSTUVWXYZ !!! ???";
	//getline(cin, s);
	cout << encryption(s) << endl << decryption(encryption(s));
	return 0;
}