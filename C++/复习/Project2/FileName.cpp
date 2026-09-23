#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

inline void swap(double& x, double& y) {
	double z = x;
	x = y;
	y = z;
}

int main() {
	srand(time(0));
	vector<double> score(10);
	for (auto& i : score) {
		i = rand() % 100;
	}
	for (int i = 0; i < score.size() - 1; i++) {
		bool swapped = false;
		for (int j = 0; j < score.size() - i - 1; j++) {
			if (score[j] < score[j + 1]) {
				swap(score[j], score[j + 1]);
				swapped = true;
			}
		}
		if (!swapped) break;
	}
	for (auto& i: score) {
		cout << i << endl;
	}
	return 0;
}