/*
2026年3月10日13点42分
王新疆
*/

#include <iostream>

using namespace std;

class Integer {
private:
	int value;
public:
	Integer();
	Integer(int);
	int Getvalue() const;
	bool Isprime() const;
	bool Isequal(int) const;
	bool Isequal(Integer&) const;
	Integer Add(Integer&);
	friend Integer Add(Integer&, int);
};

Integer::Integer() :value(0) {}

Integer::Integer(int x) :value(x) {}

int Integer::Getvalue() const {
	return value;
}

bool Integer::Isprime() const {
	int n = this->Getvalue();

	if (n <= 1) {
		return false;
	}
	if (n == 2 || n == 3) {
		return true;
	}
	if (n % 2 == 0 || n % 3 == 0) {
		return false;
	}

	int limit = (int)sqrt(n);
	for (int i = 5; i <= limit; i += 6) {
		if (n % i == 0 || n % (i + 2) == 0) {
			return false;
		}
	}
	return true;
}

bool Integer::Isequal(int x) const {
	if (this->value == x) {
		return true;
	}
	else return false;
}

bool Integer::Isequal(Integer& x) const{
	if (this->Getvalue() == x.Getvalue()) {
		return true;
	}
	else return false;
}

Integer Integer::Add(Integer& x) {
	return Integer(this->value + x.value);
}

Integer Add(Integer& a, int b) {
	return Integer(a.value + b);
}

int main() {
	Integer x(2019);
	cout << x.Isprime() << endl;
	
	int a;
	cout << "请输入一个整数，" << endl;
	cin >> a;
	if (x.Isequal(a)) {
		cout << "这个整数和x相等！" << endl;
	}
	else {
		cout << "这个整数和x不相等！" << endl;
	}

	Integer y(2109); 
	cout << y.Isequal(x) << endl;

	cout << "x和y的和是：" << x.Add(y).Getvalue() << endl 
		<< "x和119的和是：" << Add(x, 119).Getvalue() << endl;

}