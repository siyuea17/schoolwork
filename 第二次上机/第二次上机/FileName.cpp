#include <iostream>
#include <vector>
using namespace std;
int main() {
	vector<vector<int>> v1(5, vector<int>(5, 0));
	int i(0), j(0), number1(3);
	int* ptr = nullptr;
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 5; j++) {
			ptr = &v1[i][j];
			*ptr = number1++;
			cout << *ptr << '\t';
		}
		cout << endl;
	}
	cout << endl;
	//接下来把矩阵的右上部分变成0
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 5; j++) {
			ptr = &v1[i][j];
			if (i < j) *ptr = 0;
			cout << *ptr << '\t';
		}
		cout << endl;//输出一下看看结果
	}
	return 0;
}
//在给指针初始化之后忘记了在循环内更新指针导致指针一直指向第一个元素
//错误地把v1[i][j]的地址给赋值给了*ptr