/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw13_02
* Description: 点类
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/3/19 17:39:27
*/

#include <iostream>
#include <cmath>

using namespace std;

class Point {
private:
	double x, y;
public:
	Point() : x(0), y(0) {}
	Point(double x, double y) : x(x), y(y) {}
	~Point() = default;

	double getX() const { return x; }
	double getY() const { return y; }

	double dist(const Point& p) const {
		double dx = x - p.x;
		double dy = y - p.y;
		return sqrt(dx * dx + dy * dy);
	}
};

class Point3D : public Point {
private:
	double z;
public:
	Point3D() : Point(), z(0) {}
	Point3D(double x, double y, double z) : Point(x, y), z(z) {}
	~Point3D() = default;

	double getZ() const { return z; }
	double dist(const Point3D& p) const {
		double dx = getX() - p.getX();
		double dy = getY() - p.getY();
		double dz = z - p.z;
		return sqrt(dx * dx + dy * dy + dz * dz);
	}
};

int main() {
	Point A1(0, 0);
	Point A2(4.5, 6.7);
	cout << "A1(0, 0) 与 A2(4.5, 6.7) 的距离 = " << A1.dist(A2) << endl;

	Point3D B1(0, 0, 0);
	Point3D B2(4.5, 6.7, 8);
	cout << "B1(0, 0, 0) 与 B2(4.5, 6.7, 8) 的距离 = " << B1.dist(B2) << endl;

	return 0;
}