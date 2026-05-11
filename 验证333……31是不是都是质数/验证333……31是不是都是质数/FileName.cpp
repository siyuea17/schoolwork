#include <iostream>
#include <string>
using namespace std;
bool isPrime3(int n) {
	// 快速排除常见情况
	if (n < 2) return false;
	if (n == 2 || n == 3) return true;
	if (n % 2 == 0 || n % 3 == 0) return false;

	// 优化：只需要检查到 sqrt(n)
	int limit = sqrt(n);
	for (int i = 5; i <= limit; i += 6) {
		if (n % i == 0 || n % (i + 2) == 0) {
			return false;
		}
	}
	return true;
}

int main() {
	long long i(31), j(2);
	for (long long k = 1; k <= 1e2; k++) {
		i = 3 * pow(10, j) + i;
		j++;
		if (!isPrime3(i)) cout << i << "                    " << isPrime3(i) << endl;
	}
}