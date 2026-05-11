#include <iostream>
#include <vector>
using namespace std;
int main() {
	int n, m;
	cin >> n >> m;
	const int cn = n, cm = m;
	vector<bool> v1(cn, 1);
	int i(0), k(0);
	k = n;
	while (k > 1) {
		if (v1[i]) {//如果这个人没有出局
			m--;//这个人报数，剩余要报数的人减一
			if (m == 0) {//如果报数从1报到m了
				v1[i] = 0;//这个人出圈
				k--;//总人数减一
				m = cm;//更新一下m
			}
			else i++;//如果还没从1报到m，那看下一个人
		}
		else i++;//如果这个人出局了，那么看下一个人
		if (i == v1.size()) i = 0;//i等于10了就重置成0，重新从第一个人开始报数
	}
	for (int j = 0; j < n;j++) {
		if (v1[j] == 1) {
			cout << j + 1;
			break;
		}
	}
	return 0;
}