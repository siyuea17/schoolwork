#include <vector>
using namespace std;
class Solution {
public:
    /**
     * @param nums: an array
     * @param k: an integer
     * @return: the maximum average value
     */
    double findMaxAverage(vector<int>& nums, int k) {
        if (nums.empty() || k <= 0 || k > nums.size()) return 0.0;

        long long windowSum = 0;
        for (int i = 0; i < k; ++i) windowSum += nums[i];
        long long maxSum = windowSum;

        for (int i = k; i < nums.size(); ++i) {
            windowSum += nums[i] - nums[i - k];
            if (windowSum > maxSum) maxSum = windowSum;
        }
        return static_cast<double>(maxSum) / k;
    }
};