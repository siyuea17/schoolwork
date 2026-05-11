/*
* Copyright (c) 2026,Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename:hw11_01.cpp
* Description:矩形类
*
* Version:1.0
* Author:王新疆
* Date:2026年3月17日17点41分
*/

#include <iostream>

class Rectangle2D {
public:
    Rectangle2D()
        : x(0), y(0), width(1), height(1) {}

    Rectangle2D(double x, double y, double width, double height)
        : x(x), y(y), width(width), height(height) {}

    ~Rectangle2D() {}

    Rectangle2D(const Rectangle2D& other)
        : x(other.x), y(other.y), width(other.width), height(other.height) {
    }

    Rectangle2D& operator=(const Rectangle2D& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
        }
        return *this;
    }

    Rectangle2D(Rectangle2D&& other) noexcept
        : x(other.x), y(other.y), width(other.width), height(other.height) {
        other.x = 0;
        other.y = 0;
        other.width = 0;
        other.height = 0;
    }

    Rectangle2D& operator=(Rectangle2D&& other) noexcept {
        if (this != &other) {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            other.x = 0;
            other.y = 0;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }

    double getArea() const{
        return width * height;
    }

    bool contains(double x0,double y0) const {
        if (x - width / 2 < x0 && x0 < x + width / 2 && y - height / 2 < y0 && y0 < y + height / 2) {
            return true;
        }
        else {
            return false;
        }
    }

    bool contains(const Rectangle2D& r) const {
        if (r.width > width || r.height > height) {
            return false;
        }
        if ((r.x - r.width / 2 >= x - width / 2) &&
            (r.x + r.width / 2 <= x + width / 2) &&
            (r.y - r.height / 2 >= y - height / 2) &&
            (r.y + r.height / 2 <= y + height / 2)){
            return true;
        }
        else {
            return false;
        }
    }

    bool overlaps(const Rectangle2D& r) const {
        bool xOverlap = (x + width / 2 >= r.x - r.width / 2) &&
            (r.x + r.width / 2 >= x - width / 2);
        bool yOverlap = (y + height / 2 >= r.y - r.height / 2) &&
            (r.y + r.height / 2 >= y - height / 2);
        return xOverlap && yOverlap;
    }

private:
    double x;
    double y;
    double width;
    double height;
};   

int main() {
    Rectangle2D r1(2, 2, 5.4, 4.8), r2(4, 5, 10.6, 3.3), r3(3, 5, 2.2, 5.5);
    std::cout << r1.getArea() << " " << r1.contains(3, 3) << " " << r1.contains(r2) << " " << r1.overlaps(r3);
}