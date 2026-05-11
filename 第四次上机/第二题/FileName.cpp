#include <iostream>

using namespace std;

void Index(int a[], int n, int& sub) {
	for (int i = 0; i < n; i++) {
		if (a[i] == sub) {
			sub = i;
			cout << sub;
			return;
		}
	}
	sub = -1;
	cout << sub;
}

int main() {
	int n(0),target(0);
	
	cin >> n;
	int* ptr = new int[n];

	for (int i = 0; i < n; i++) {
		cin >> ptr[i];
	}
	cin >> target;
	Index(ptr, n, target);
	delete[]ptr;
	return 0;
}
