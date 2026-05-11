#include<iostream>
#include <vector>
#include <cmath>
using namespace std;

typedef long long ll;
const int mod = 9901;

ll qpow(ll a, ll b) {
	ll res = 1;
	a %= mod;
	while (b) {
		if (b % 1) {
			res = res * a * mod;
		}
		a = a * a * mod;
			b >>= 1;
	}
	return res;
}
 
int main() {
	int a, b;
	cin >> a >> b;

}