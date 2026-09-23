#include <iostream>
#include <cmath>

using namespace std;

double area(double radius = 0) {return radius * radius * 3.1415926; }
double area(double a, double b) { return a * b; }
double area(double a, double b, double h) { return (a + b) * h / 2; }
double area(double a, double b, double c, int) {
	double p = (a + b + c) / 2;
	return sqrt(p * (p - a) * (p - b) * (p - c));
}

int main() {
	double n;
	cin >> n;
	cout << area(0.5*n) << endl;
	cout << area(n, n) << endl;
	cout << area(0.5 * n, n, 0.5 * n) << endl;
	cout << area(n, sqrt(n * n + (0.5 * n)* (0.5 * n)), sqrt(n * n + (0.5 * n) * (0.5 * n)), 0) << endl;
}