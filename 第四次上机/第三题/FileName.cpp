#include <iostream>

using namespace std;

class classscore {
private:
	int n;//学生总数
	int count;//完成学生总数
	int* ptID;//学生id
	float* stu;//学生成绩
	bool isStat;//是否完成标识
public:
	classscore() :n(0), count(0), ptID(nullptr),stu(nullptr),isStat(false)
	{ cout << "classScore类已创建" << endl; }

	void input(int n);
	int stat();
	void show();
	void print();
};
void classscore::input(int n) {
	classscore::n = n;
	isStat = false;

	if (ptID != nullptr || stu != nullptr) {
		delete[]ptID;
		delete[]stu;
	}
	
	ptID = new int[n];
	stu = new float[n];
	
	cout << "请输入学生编号" << endl;
	for (int i = 0; i < n; i++) {
		cin >> ptID[i];
	}
	
	cout << "请输入学生成绩" << endl;
	for (int i = 0; i < n; i++) {
		cin >> stu[i];
	}
}
int classscore::stat() {
	if (n == 0 || ptID == nullptr || stu == nullptr) {
		cout << "请先调用input" << endl;
		return 1;
	}
	
	count = 0;
	for (int i = 0; i < n; i++) {
		if (stu[i] >= 80) {
			count += 1;
		}
	}
	isStat = true;
	cout << "统计完成" << endl;
	return 0;
}
void classscore::show() {
	if (!isStat) {
		cout << "未完成统计！" << endl;
		return;
	}

	cout << "符合条件的人员信息为：" << endl;
	for (int i = 0; i < n; i++) {
		if (stu[i] >= 80.0) {
			cout << ptID[i] << "\t" << stu[i] << endl;
		}
	}
}
void classscore::print() {
	if (!isStat) {
		cout << "未完成统计！" << endl;
		return;
	}
	cout << "符合条件的总人数为" << count << endl;
}
int main() {
	classscore class1;
	cout << "请输入学生总数" << endl;
	
	int num(0);
	cin >> num;
	
	class1.input(num);
	class1.stat();
	class1.print();
	class1.show();
	return 0;
}