#pragma once
#include <fstream>
#include <string>
#include <vector>

class fireCampChain :public std::fstream
{
protected:
	std::fstream file;
	std::vector<std::string> barracksDistributionMap;

	int rows;
	int cols;
	bool isValid(int x, int y) const;
	void ignite(int x, int y);
	void display() const;
public:
	void fire(const std::string inmap, const std::string outmap, const int x, const int y);
	virtual ~fireCampChain();
};