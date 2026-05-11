#include <iostream>
#include <string>
using namespace std;
int main() {
	string target;
	cout << "请给出一个单词：";
	cin >> target;
	system("cls");
	int length = target.length();
	string attempt(length, '*'),badchars;
	int guesses = 5;
	cout << "单词已经准备好，它有" << length << "个字母：" << attempt << endl;
	do {
		char letter;
		cout << "请猜测一个字母：";
		cin >> letter;
		if (badchars.find(letter) != string::npos || attempt.find(letter) != string::npos) {
			cout << "已经猜过该字母，请重猜" << endl;
			continue;
		}
		auto loc = target.find(letter);
		if (loc == string::npos) {
			cout << "没有该字母！" << endl;
			--guesses;
			badchars += letter;
		}
		else {
			cout << "有这个字母，继续加油！" << endl;
			do {
				attempt[loc] = letter;
				loc = target.find(letter, loc + 1);

			} while (loc != string::npos);
		}
		cout << "你猜测的单词：" << attempt << endl;
		if (attempt != target) {
			cout << "剩余" << guesses << "次猜错机会" << endl;
		}
	} while (guesses > 0 && attempt != target);
	if (guesses > 0) {
		cout << "成功了，恭喜你！" << endl;
	}
	else {
		cout << "对不起，失败了，下次再挑战吧，单词是：" << target << endl;
	}
	return 0;
}