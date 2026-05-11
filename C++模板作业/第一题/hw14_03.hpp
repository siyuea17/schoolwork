/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: hw14_03
* Description: 二维点类和三维点类，三维点公有继承二位点类
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/4/2 12:45:02
*/

#pragma once
#include <iostream>
#include <cmath>
class Point2D {
public:
	Point2D(double x = 0, double y = 0) :x(x), y(y) {}
	Point2D(const Point2D& other) :x(other.x), y(other.y) {}
	virtual ~Point2D() = default;
	virtual double dist(const Point2D& other) const;
protected:
	double x, y;
};

double Point2D::dist(const Point2D& other) const {
	return std::sqrt((this->x - other.x) * (this->x - other.x) + (this->y - other.y) * (this->y - other.y));
}

class Point3D :public Point2D {
protected:
	double z;
public:
	Point3D(double x = 0, double y = 0, double z = 0):Point2D(x,y), z(z) {}
	Point3D(const Point3D& other):Point2D(other),z(other.z) {}
	virtual ~Point3D() = default;
	virtual double dist(const Point3D& other) const;
};

double Point3D::dist(const Point3D& other) const {
	return std::sqrt((this->x - other.x) * (this->x - other.x) + (this->y - other.y) * (this->y - other.y) +(this->z - other.z) * (this->z - other.z));
}

template <typename T1>
double mydist(const T1& a, const T1& b) {
	return a.dist(b);
}

int main() {
	Point2D A1(1.2, 3.4), A2(5.6, 7.8);
	Point3D B1(1.2, 3.4, 5.6), B2(9.8, 7.6, 5.4);
	std::cout << mydist(A1, A2) << std::endl;
	std::cout << mydist(B1, B2) << std::endl;

	return 0;
}