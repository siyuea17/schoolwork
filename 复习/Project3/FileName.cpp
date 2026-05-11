#include <iostream>

using namespace std;

class CCircle {
	double SetRadius;
	double x, y;
public:
	CCircle():SetRadius(0){}
	~CCircle();
	double area() {
		return 3.1415 * SetRadius * SetRadius;
	}
	double zhouchang() {
		return 3.1415 * SetRadius * 2;
	}

};


int main() {


}