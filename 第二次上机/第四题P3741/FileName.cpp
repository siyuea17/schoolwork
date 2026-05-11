#include <iostream>
#include <string>
#include <vector>
using namespace std;
int main() {
    int length;
    string input;
    cin >> length >> input;
    int count(0), count_now(0);
    //先计算原始字符串中的VK数量
    for (int i = 0; i + 1 < length; i++) {
        if (input[i] == 'V' && input[i + 1] == 'K') {
            count += 1;
        }
    }
    string copy;
    vector<int> count_v(length);
    for (int i = 0; i < length; i++) {
        copy = input;
        count_now = 0;
        copy[i] = (copy[i] == 'V') ? 'K' : 'V';
        for (int i = 0; i + 1 < length; i++) {//把里面的每一个字符都改变改变一次看看VK的数量变化
            if (copy[i] == 'V' && copy[i + 1] == 'K') {
                count_now += 1;
            }
        }
        count_v[i] = count_now;//把修改各个位置之后VK的数量进行记录
    }
    for (int i = 0; i < length; i++) {
        if (count_v[i] > count) {
            cout << count + 1;
            return 0;//如果有变换使VK数量变多，直接输出原VK数量加一，结束程序
        }
    }
    cout << count;//不然就输出原VK数量
    return 0;
}