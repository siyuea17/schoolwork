#include <iostream>
#include <vector>
using namespace std;
void fun(int *ptr,int n , int m){
	for (int i = 0; i < n; i++) {
		*ptr = *(ptr - m);//把所有元素往后移动m个位置
		ptr-=1;

	}//此时指针只向第m+1个元素，也就是输入的第一个元素
	ptr-=m - 1;//指向第一个元素
	for (int i = 0; i < m; i++) {
		*ptr = *(ptr + n);
		ptr += 1;
	}

}
int main() {
	int n(0), m(0);
	cin >> n >> m;
	vector<int> v(n+m);
	for (int i = 0; i < n; i++) {
		cin >> v[i];
	}
	int *ptr = &v[n + m - 1];//一个指向第n+m个元素的指针
	fun(ptr, n, m);
	for (int i = 0; i < n; i++) {
		cout << v[i] << " ";
	}
	return 0;
}
/*
#include <iostream>
#include <vector>
#include <algorithm> // 用于reverse函数
using namespace std;

int main() {
    int n, m;
    cin >> n >> m;
    vector<int> v(n);
    
    for (int i = 0; i < n; i++) {
        cin >> v[i];
    }
    
    // 循环右移m位
    m = m % n; // 处理m大于n的情况
    reverse(v.begin(), v.end()); // 整体反转
    reverse(v.begin(), v.begin() + m); // 反转前m个元素
    reverse(v.begin() + m, v.end()); // 反转剩余元素
    
    for (int i = 0; i < n; i++) {
        cout << v[i] << " ";
    }
    
    return 0;
}
*/