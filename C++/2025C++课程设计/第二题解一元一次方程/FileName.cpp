#include <iostream>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

double solve(const string& e) {
    double num = 0;
    double const_num = 0;//最终常数项
    double coefficient = 0;//最终系数
    char unknown = 'x';//未知数字母
    int side = 1;//正负数
    int sign = 1;//等号左边或者右边
    bool is_number = false;//判断之前的数字有没有结算

    string equation = e + '+';
    
    for (int i = 0; i < equation.size(); i++) {
        char ch = equation[i];
        if (ch >= '0' && ch <= '9') {
            num = num * 10 + (ch - '0');
            is_number = true;
        }
        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            unknown = ch;
            coefficient += (is_number ? num : 1) * side * sign;
            num = 0;
            is_number = false;
        }
        else if (ch == '+' || ch == '-' || ch == '=') {
            if (is_number) {// 处理常数项
                const_num += side * sign * num;
                num = 0;
                is_number = false;
            }
            if (ch == '+') sign = 1;// 更新符号
            else if (ch == '-') sign = -1;
            else if (ch == '=') {
                side = -1;
                sign = 1;
            }
        }
        else if (ch == ' ') {// 忽略空格
            continue;
        }
    }
    if (coefficient == 0) { // 解方程
        if (const_num == 0) return 0;// 无穷解
        else return NAN;// 无解
    }
    return - const_num / coefficient;
}
int main() {
    cout << "请输入方程：" << endl;
    string equation;
    getline(cin, equation);
	cout << "解：" << fixed << setprecision(3) << solve(equation) << endl;
    return 0;
}