#include <iostream>
using namespace std;

int main() {
	int kind = 1;
	int m, n;
	cin >> n >> m;
	int* ptr = new int[n];
	for (int i = 0; i < n; i++) {
		ptr[i] = i;
	}
	int a, b;
	for (int i = 0; i < m; i++) {
		cin >> a >> b;
		ptr[a] = ptr[b];
	}
	for (int i = 0; i < n; i++) {
		if (ptr[i] != ptr[0]) {
			kind++;
		}
	}
	cout << kind;
}