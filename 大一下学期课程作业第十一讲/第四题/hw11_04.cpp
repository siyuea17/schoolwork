/*
* Copyright (c) 2026,Wuhan CUG Co.,Ltd
* All rights reserved.
*
* Filename:hw11_04.cpp
* Description:矩阵的加法和乘法
*
* Version:1.0
* Author:王新疆
* Date:2026年3月17日17点40分
*/

#include <iostream>
#include <vector>

using namespace std;
using matrix = vector<vector<double>>;

matrix matrixSum(const matrix& v1, const matrix& v2) {
	if (v1.empty() && v2.empty()) {
		return {};
	}
	if (v1.empty() || v2.empty()) {
		throw invalid_argument("存在空矩阵，无法相加！");
	}
	if (v1.size() != v2.size() || v1[0].size() != v2[0].size()) {
		throw invalid_argument("非同型矩阵不能相加！");
	}
	
	if (v1.size() == v2.size() && v1[0].size() == v2[0].size()) {
		size_t i, j, m, n;
		m = v1.size();
		n = v1[0].size();
		matrix res(m, vector<double>(n));
		for (i = 0; i < m; i++) {
			for (j = 0; j < n; j++) {
				res[i][j] = v1[i][j] + v2[i][j];
			}
		}
		return res;
	}
}

matrix operator+(const matrix& v1, const matrix& v2) {
	return matrixSum(v1, v2);
}

matrix matrixMultiplication(const matrix& v1, const matrix& v2) {
	if (v1.empty() || v2.empty()) {
		throw invalid_argument("存在空矩阵，无法相乘！");
	}
	if (v1[0].size() != v2.size()) {
			throw std::invalid_argument("矩阵维度不匹配，无法相乘！");
		}
	else {
		size_t i, j, k;
		size_t rows_v1, n, cols_v2;
		rows_v1 = v1.size();
		n = v1[0].size();
		cols_v2 = v2[0].size();

		matrix res(rows_v1, vector<double>(cols_v2));

		for (i = 0; i < rows_v1; i++) {
			for (j = 0; j < cols_v2; j++) {
				for (k = 0; k < n; k++) {
					res[i][j] += v1[i][k] * v2[k][j];
				}
			}
		}
		return res;
	}
}

matrix operator*(const matrix& v1, const matrix& v2) {
	return matrixMultiplication(v1, v2);
}