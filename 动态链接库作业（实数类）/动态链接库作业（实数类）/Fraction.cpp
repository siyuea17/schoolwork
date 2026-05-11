#include "pch.h"
#include "Fraction.h"

#include <stdexcept>
#include <cmath>

static int gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

Fraction::Fraction(int num, int den) : num_(num), den_(den) {
    if (den_ == 0) {
        throw std::invalid_argument("分母不能为零！");
    }
    if (den_ < 0) {
        num_ = -num_;
        den_ = -den_;
    }
    reduce();
}

Fraction::~Fraction() = default;

void Fraction::reduce() {
    int g = gcd(num_, den_);
    num_ /= g;
    den_ /= g;
}

int Fraction::numerator() const {
    return num_;
}

int Fraction::denominator() const {
    return den_;
}

Fraction& Fraction::operator=(const Fraction& other) {
    if (this != &other) {
        num_ = other.num_;
        den_ = other.den_;
    }
    return *this;
}

Fraction Fraction::operator+(const Fraction& other) const {
    int new_num = num_ * other.den_ + other.num_ * den_;
    int new_den = den_ * other.den_;
    return Fraction(new_num, new_den);
}

Fraction Fraction::operator-(const Fraction& other) const {
    int new_num = num_ * other.den_ - other.num_ * den_;
    int new_den = den_ * other.den_;
    return Fraction(new_num, new_den);
}

Fraction Fraction::operator*(const Fraction& other) const {
    int new_num = num_ * other.num_;
    int new_den = den_ * other.den_;
    return Fraction(new_num, new_den);
}

Fraction Fraction::operator/(const Fraction& other) const {
    if (other.num_ == 0) {
        throw std::invalid_argument("分母不能为零！");
    }
    int new_num = num_ * other.den_;
    int new_den = den_ * other.num_;
    return Fraction(new_num, new_den);
}

Fraction& Fraction::operator+=(const Fraction& other) {
    *this = *this + other;
    return *this;
}

Fraction& Fraction::operator-=(const Fraction& other) {
    *this = *this - other;
    return *this;
}

Fraction& Fraction::operator*=(const Fraction& other) {
    *this = *this * other;
    return *this;
}

Fraction& Fraction::operator/=(const Fraction& other) {
    *this = *this / other;
    return *this;
}

bool Fraction::operator==(const Fraction& other) const {
    return num_ == other.num_ && den_ == other.den_;
}

bool Fraction::operator!=(const Fraction& other) const {
    return !(*this == other);
}

bool Fraction::operator<(const Fraction& other) const {
    return num_ * other.den_ < other.num_ * den_;
}

bool Fraction::operator<=(const Fraction& other) const {
    return *this < other || *this == other;
}

bool Fraction::operator>(const Fraction& other) const {
    return !(*this <= other);
}

bool Fraction::operator>=(const Fraction& other) const {
    return !(*this < other);
}

Fraction Fraction::operator-() const {
    return Fraction(-num_, den_);
}

Fraction::operator double() const {
    return static_cast<double>(num_) / den_;
}

std::ostream& operator<<(std::ostream& os, const Fraction& f) {
    os << f.num_;
    if (f.den_ != 1) {
        os << '/' << f.den_;
    }
    return os;
}

std::istream& operator>>(std::istream& is, Fraction& f) {
    int n, d = 1;
    char slash;
    is >> n;
    if (is.peek() == '/') {
        is >> slash >> d;
    }
    f = Fraction(n, d);
    return is;
}