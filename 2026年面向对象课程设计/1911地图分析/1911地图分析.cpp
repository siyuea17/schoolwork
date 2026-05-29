/*
* Copyright (c) 2026, Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename: _3779创建Animals类
* Description: 你现在手里有一份大小为 N x N 的网格grid，上面的每个单元格都用 0 和 1 标记好了。其中 0 代表海洋，1 代表陆地，请你找出一个海洋单元格，这个海洋单元格到离它最近的陆地单元格的距离是最大的。
*              我们这里说的距离是「曼哈顿距离」（ Manhattan Distance）：(x0, y0) 和 (x1, y1) 这两个单元格之间的距离是 |x0 - x1| + |y0 - y1| 。
*              如果网格上只有陆地或者海洋，请返回 -1。
*
* Version: 1.0
* Author: 王新疆
* Date: 2026/5/15 20:33:27
*/

#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

class Solution {
public:
    /**
     * @param grid: An array.
     * @return: An integer.
     */
    int maxDistance(vector<vector<int>>& grid) {
        // Write your code here.
		int n = grid.size();
        if (n==0) {
            return -1;
		}
		queue<pair<int, int>> q;
		vector<vector<int>> dist(n, vector<int>(n, -1));
        int landCount = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (grid[i][j] == 1) {
                    dist[i][j] = 0;
					q.push({ i, j });
					++landCount;
                }
            }
        }

        if (landCount == 0 || landCount == n * n) {
            return -1;
		}

		int dx[4] = { -1, 1, 0, 0 };
		int dy[4] = { 0, 0, -1, 1 };
		int MaxDist = 0;

        while (!q.empty()) {
            int x = q.front().first;
            int y = q.front().second;
            q.pop();
            for (int d = 0; d < 4; ++d) {
                int nx = x + dx[d];
                int ny = y + dy[d];
                if (nx >= 0 && nx < n && ny >= 0 && ny < n && dist[nx][ny] == -1) {
                    dist[nx][ny] = dist[x][y] + 1;
                    MaxDist = max(MaxDist, dist[nx][ny]);
                    q.push({ nx, ny });
                }
            }
        }

		return MaxDist;
    }
};