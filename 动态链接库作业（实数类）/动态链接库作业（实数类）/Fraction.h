#pragma once

// 定义导出宏
#ifdef MY_EXPORTS
#define FRACTION_API __declspec(dllexport)
#else
#define FRACTION_API __declspec(dllimport)
#endif // MY_EXPORTS

#include <iostream>

class FRACTION_API Fraction {
public:
    // 构造函数
    Fraction(int num = 0, int den = 1);
    virtual ~Fraction();

    // 获取分子/分母（只读）
    int numerator() const;
    int denominator() const;

    // 赋值运算符
    Fraction& operator=(const Fraction& other);

    // 算术运算符
    Fraction operator+(const Fraction& other) const;
    Fraction operator-(const Fraction& other) const;
    Fraction operator*(const Fraction& other) const;
    Fraction operator/(const Fraction& other) const;

    // 复合赋值运算符
    Fraction& operator+=(const Fraction& other);
    Fraction& operator-=(const Fraction& other);
    Fraction& operator*=(const Fraction& other);
    Fraction& operator/=(const Fraction& other);

    // 比较运算符
    bool operator==(const Fraction& other) const;
    bool operator!=(const Fraction& other) const;
    bool operator<(const Fraction& other) const;
    bool operator<=(const Fraction& other) const;
    bool operator>(const Fraction& other) const;
    bool operator>=(const Fraction& other) const;

    // 取负
    Fraction operator-() const;

    // 类型转换：转为 double
    explicit operator double() const;

    // 输入输出重载
    friend FRACTION_API std::ostream& operator<<(std::ostream& os, const Fraction& f);
    friend FRACTION_API std::istream& operator>>(std::istream& is, Fraction& f);

private:
    int num_;
    int den_;

    // 约分
    void reduce();
};

static int gcd(int a, int b);

