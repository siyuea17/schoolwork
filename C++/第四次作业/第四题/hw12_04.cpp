/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw12_04
* Description: 多项式类PolyInt,实现了多项式的加法运算
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/24 20:34:29
*/

#include <iostream>
#include <algorithm>

using namespace std;

class PolyInt {
public:
	PolyInt() :n(0), a(new int[1]) { a[0] = 0; }

	PolyInt(int n) :n(n), a(new int[n + 1]) {
		for (int i = 0; i <= n; ++i) a[i] = 0;
	}

	PolyInt(int n, int a[]) :n(n), a(new int[n + 1]) {
		for (int i = 0; i <= n; ++i) this->a[i] = a[i];
	}

	PolyInt(const PolyInt& other) :n(other.n), a(new int[n + 1]) {
		for (int i = 0; i <= n; ++i)  a[i] = other.a[i];
	}

	//移动构造函数
	PolyInt(PolyInt&& other) noexcept :n(other.n), a(other.a) {
		other.n = 0;
		other.a = nullptr;
	}

	void Display() const {
		cout << "[";
		for (int i = 0; i < n; ++i) { cout << a[i] << ","; }
		cout << a[n];
		cout << "]" << endl;
	}

	PolyInt& operator=(const PolyInt& other) {
		if (this != &other) {
			delete[] a;
			n = other.n;
			a = new int[n + 1];
			for (int i = 0; i <= n; ++i) a[i] = other.a[i];
		}
		return *this;
	}

	//移动赋值运算符
	PolyInt& operator=(PolyInt&& other) noexcept {
		if (this != &other) {
			delete[] a;
			n = other.n;
			a = other.a;
			other.n = 0;
			other.a = nullptr;
		}
		return *this;
	}

	friend PolyInt operator+(const PolyInt& p, const PolyInt& q) {
		PolyInt res(max(p.n, q.n));
		for (int i = 0; i <= res.n; ++i) {
			int temp_p = (i <= p.n) ? p.a[i] : 0;
			int temp_q = (i <= q.n) ? q.a[i] : 0;
			res.a[i] = temp_p + temp_q;
		}
		return res;
	}

	virtual ~PolyInt() { delete[] a; }

private:
	int n, * a;
};

int main() {
	int arr1[] = { 1,2,0,4 };
	PolyInt p(3, arr1);
	int arr2[] = { 2, 0, 4, 3, 0, 5 };
	PolyInt q(5, arr2);
	PolyInt s(p + q);
	return 0;
}