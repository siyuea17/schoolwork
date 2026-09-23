#include <iostream>
using namespace std;
int main() {
	int n;
	cin >> n;
	int i(0), j(0),count(0);
	for (i = 3; i <= n; i++) {
		for (j = 2; j < i; j++) {
			if (i % j == 0) {
				count += 1;
				break; 
			}
		}
	}
	cout << n - 1 - count;
	return 0;
}