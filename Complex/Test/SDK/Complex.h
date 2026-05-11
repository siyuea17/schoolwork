#pragma once
#ifdef MYDLL_EXPORTS
#define MYCLASSDLL_EXP __declspec(dllexport)
#else
#define MYCLASSDLL_EXP __declspec(dllimport)
#endif
class MYCLASSDLL_EXP Complex {
public:
	friend Complex MYCLASSDLL_EXP operator+(Complex& p1, Complex& p2);
	friend Complex MYCLASSDLL_EXP operator+(Complex& p, float n);
	friend Complex MYCLASSDLL_EXP operator+(float n, Complex& p);
	friend Complex MYCLASSDLL_EXP operator/(Complex& p1, Complex& p2);
	Complex();
	Complex(float x, float y);
	void Display();
	Complex operator+=(Complex& p);
private:
	float x;
	float y;
};


