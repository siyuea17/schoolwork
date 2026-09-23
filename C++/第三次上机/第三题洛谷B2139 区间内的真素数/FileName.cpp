#include <iostream>
#include <string>
#include <cmath>
using namespace std;
bool zhishu(int a) {
	if (a <= 1) return false;
	if (a <= 3) return true;
	if (a % 2 == 0 || a % 3 == 0) return false;
	int limit = sqrt(a);
	for (int i = 5; i <= limit; i += 6) {
		if (a % i == 0 || a % (i + 2) == 0) {
			return false;
		}
	}
	return true;
}
int fanzhuan(int a) {
	int b(0);
	while (a != 0) {
		b = b * 10 + a % 10;
		a = a / 10;
	}
	return b;
}
string result;
int main() {
	int m(0), n(0);
	cin >> m >> n;
	for (int i = m; i <= n; i++) {
		if (zhishu(i) && zhishu(fanzhuan(i))) {
			result += to_string(i) + ",";
		}
	}
	if (!result.empty()) {
		result.pop_back();
		cout << result;
	}
	else {
		cout << "No";
	}
	return 0;
}