#include <iostream>
#include <vector>
using namespace std;

int main() {
    int n(0), i(0), j(0), k(0);
    cin >> n;

    vector<string> v =
    { "001","002","010","011","023","025","031","036","037","102","325","438" };//编号
    vector<double> score =
    { 13.6,14.8,12.4,12.0,15.6,13.4,14.9,12.6,13.4,12.5,15.1,12.7 };//成绩
    vector<string> input(n); //用户输入的编号
    vector<string> v_chosen(n);//用来放被选择的用户输入的编号,来和被选择的成绩对应
    vector<double> score_chosen(n);//用来放被选择的编号对应的成绩

    for (; i < n; i++) {
        cin >> input[i];
    }
    // 重置i和j
    i = 0;
    j = 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < 12; j++) {
            if (input[i] == v[j]) {//如果发现了用户的输入编号
                v_chosen[k] = v[j];//放编号
                score_chosen[k] = score[j];//放成绩
                k++;//准备放下一个
                break;//出循环
            }
        }
    }
    //开始排序
    bool order_finished;
    double temp_score;
    string temp_v;
    for (i = 0; i < k - 1; i++) {
        order_finished = false;//没排好
        for (j = 0; j < k - i - 1; j++) {
            if (score_chosen[j] > score_chosen[j + 1]) {
                //交换成绩
                temp_score = score_chosen[j];
                score_chosen[j] = score_chosen[j + 1];
                score_chosen[j + 1] = temp_score;
                //同时交换编号
                temp_v = v_chosen[j];
                v_chosen[j] = v_chosen[j + 1];
                v_chosen[j + 1] = temp_v;

                order_finished = true;
            }
        }
        // 如果没有发生交换,说明已经排好了,出循环
        if (!order_finished) {
            break;
        }
    }
    //输出结果
    for (i = 0; i < k; i++) {
        cout << v_chosen[i] << "\t\t" << score_chosen[i] << endl;
    }
    return 0;
}