#include <iostream>

template <typename T>
T getMax(&T a, &T b) {
	return a > b ? a : b;
};

