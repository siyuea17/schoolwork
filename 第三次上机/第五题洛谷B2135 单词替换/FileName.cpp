#include <iostream>
#include <vector>
#include <string>
#include <cctype>

using namespace std;

int main() {
    string s;
    getline(cin, s);
    vector<string> words;
    string temp = "";

    for (int i = 0; i < s.length(); i++) {
        if (isspace(s[i]) && !temp.empty()) {
            words.push_back(temp);
            temp = "";
        }
        else if (!isspace(s[i])) {
            temp += s[i];
        }
    }

    if (!temp.empty()) {
        words.push_back(temp);
    }

    string a, b;
    cin >> a >> b;

    for (int i = 0; i < words.size(); i++) {
        if (words[i] == a) {
            words[i] = b;
        }
    }

    for (int i = 0; i < words.size(); i++) {
        if (i == words.size() - 1) {
            cout << words[i];
        }
        else cout << words[i] << " ";
    }

    return 0;
}