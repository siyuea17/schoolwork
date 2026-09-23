#include "fireCampChain.h"
#include <iostream>
#include <vector>
#include <string>

inline fireCampChain::~fireCampChain() { file.close(); }

bool fireCampChain::isValid(int x, int y) const {
	return 0 <= x && x <= rows - 1 && 0 <= y && y <= cols - 1;
}

void fireCampChain::ignite(int x, int y) {
	int dx[] = { 0, 1, 0, -1 };
	int dy[] = { 1, 0, -1, 0 };
	if (barracksDistributionMap[x][y] == 'A') {
		barracksDistributionMap[x][y] = 'X';
		for (int i = 0; i < 4; ++i) {
			if (isValid(x + dx[i], y + dy[i])) {
				ignite(x + dx[i], y + dy[i]);
			}
		}
	}
}

void fireCampChain::display() const {
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			std::cout << barracksDistributionMap[i][j];
		}
		std::cout << std::endl;
	}
}

void fireCampChain::fire(const std::string inmap, const std::string outmap,const int x, const int y)
{
	fireCampChain fin;
	fin.open(inmap);
	if (!fin.is_open()) {
		std::cerr << "文件" << inmap << "未能成功打开";
		return;
	}

	std::string line;
	while (fin >> line) {
		barracksDistributionMap.push_back(line);
	}
	rows = barracksDistributionMap.size();
	cols = barracksDistributionMap[0].size();
	ignite(x, y);
	display();

	fireCampChain fout;
	fout.open(outmap);
	if (!fout.is_open()) {
		std::cerr << "文件" << outmap << "未能成功打开";
		return;
	}
	for (int i = 0; i < rows; ++i) {
		fout << barracksDistributionMap[i] << std::endl;
	}
}