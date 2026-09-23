#include <iostream>
#include <iomanip>

using namespace std;
bool isNum = false;
void jiafa(double a, double b, double& result) {
	result = a + b;
	isNum = true;
}
void jianfa(double a, double b ,double &result) {
	result = a - b;
	isNum = true;
}
void chengfa(double a, double b, double& result) {
	result = a * b;
	isNum = true;
}
void chufa(double a, double b, double& result) {
	if (b == 0) {
		cout << "除数不能为0" << endl;
	}
	else {
		result = a / b;
		isNum = true;
	}
}

int main() {
	double a,b,result;
	char choice;
	
	cin >> a >> choice >> b;
	switch (choice) {
	case'+':
		jiafa(a, b, result);
		break;
	case'-':
		jianfa(a, b, result);
		break;
	case'*':
		chengfa(a, b, result);
		break;
	case'/':
		chufa(a, b, result);
		break;
	default:
		cout << "无效的运算符" << endl;
	}
	if (isNum) {
		cout << "结果：" << fixed << setprecision(2) << result << endl;
	}
	return 0;
}