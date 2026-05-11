/*
2026年3月10日13点42分
王新疆
*/

#include <iostream>

using namespace std;

class Rectangle {
private:
	double width;
	double height;
public:
	Rectangle();
	Rectangle(double x, double y);
	void setwh(double, double);
	double getw() const;
	double geth() const;
	double getArea() const;
	double getPerimeter() const;
};

Rectangle::Rectangle():width(1),height(1){}

Rectangle::Rectangle(double x, double y):width(x),height(y) {}

void Rectangle::setwh(double x, double y) {
	width = x;
	height = y;
}

double Rectangle::getw() const {
	return width;
}

double Rectangle::geth() const {
	return height;
}

double Rectangle::getArea() const {
	return width * height;
}

double Rectangle::getPerimeter() const {
	return width * 2 + height * 2;
}

int main() {
	Rectangle R1(4, 40);
	Rectangle R2(3.5, 35.9);
	cout << "R1的面积是：" << R1.getArea() << "，R1的周长是：" << R1.getPerimeter() << endl;
	cout << "R2的面积是：" << R2.getArea() << "，R2的周长是：" << R2.getPerimeter() << endl;
	return 0;
}