#include <vector>
#include <algorithm>
using namespace std;
class Solution {
public:
	long long arrayGame(vector<int>& arr) {
		// write your code here
		int minNum = *min_element(arr.begin(), arr.end());
		long res = 0;
		for (int num : arr) {
			res += num - minNum;
		}
		return res;
	}
};