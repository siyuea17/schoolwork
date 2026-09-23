#include <iostream>
#include <vector>
using namespace std;
int main() {
	int n(0);
	cin >> n;
	vector<string> name(n);
	vector<float> temperature(n);
	vector<int> yn(n);
	for (int i = 0; i < n; i++) {
		cin >> name[i] >> temperature[i] >> yn[i];
	}
	int k(0);//¼ÆÊýÆ÷
	for (int i = 0; i < n; i++) {
		if (temperature[i] >= 37.5 && yn[i] == 1) {
			cout << name[i] << endl;
			k++;
		}
	}
	cout << k;
	return 0;
}