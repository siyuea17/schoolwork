#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

class point {
	double x;
	double y;
public:
	point();
	point(double a, double b);
	~point();
	double getx() const;
	double gety() const;
	double distance(point&) const;
};

point::point():x(0),y(0){}
point::point(double a, double b) :x(a), y(b) {}
point::~point(){}
double point::getx() const {
	return x;
}
double point::gety() const {
	return y;
}
double point::distance(point& another) const {
	double distance = sqrt(pow((this->x - another.x), 2) + pow((this->y - another.y), 2));
	return distance;
}


class line {
	point* ptr;
	int num;//点的个数
	double length;
	double max_x, max_y, min_x, min_y;//边界
public:
	line() :ptr(nullptr), num(0), length(0), max_x(0), max_y(0), min_x(0), min_y(0) {}
	line(int);
	~line();
	void show_length();
	void input();
	void show_AABB();
};

line::line(int n):num(n), length(0), max_x(0), max_y(0), min_x(0), min_y(0) {
	if (n > 0) {
		ptr = new point[n];
	}
	else {
		ptr = nullptr;
	}
}
void line::show_length() {
	length = 0;
	for (int i = 0; i < num - 1; i++) {
		length += ptr[i].distance(ptr[i+1]);
	}
	cout << length << endl;
}
void line::input() {
	if (num > 0 && ptr != nullptr) {
		for (int i = 0; i < num; i++) {
			double a, b;
			cin >> a >> b;
			ptr[i] = point(a, b);
		}
	}
}
line::~line() {
	if (ptr != nullptr) {
		delete[] ptr;
	}
}
void line::show_AABB() {
	if (num < 2) {
		cout << min_x << " " << min_y << endl;
		cout << max_x << " " << max_y << endl;
		return;
	}
	//
	max_x = ptr[0].getx();
	max_y = ptr[0].gety();
	min_x = ptr[0].getx();
	min_y = ptr[0].gety();
	for (int i = 0; i < num; i++) {
		max_x = max(ptr[i].getx(), max_x);
		max_y = max(ptr[i].gety(), max_y);
		min_x = min(ptr[i].getx(), min_x);
		min_y = min(ptr[i].gety(), min_y);
	}
	cout << min_x << " " << min_y << endl;
	cout << max_x << " " << max_y << endl;
}

int main() {
	int n(0);
	cin >> n;
	line line1(n);
	line1.input();
	line1.show_length();
	line1.show_AABB();
}