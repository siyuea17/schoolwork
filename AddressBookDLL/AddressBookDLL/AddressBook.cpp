#include "pch.h"
#include "AddressBook.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>


//========================================
//这是CommEntry的函数定义
//========================================
CommEntry::CommEntry()
{
}

CommEntry::~CommEntry()
{
}

void CommEntry::input(std::istream& is)
{
	if (is.rdbuf() == std::cin.rdbuf()) std::cout << "你正在输入姓名：" << std::endl;
	is >> name;

	if (is.rdbuf() == std::cin.rdbuf()) std::cout << "你正在输入电话号码：" << std::endl;
	is >> tel;
}

void CommEntry::output(std::ostream& os) const
{
	os << std::setw(15) << this->getName() << std::setw(15) << this->getTel() << std::endl;
}

std::string CommEntry::getName() const
{
	return name;
}

std::string CommEntry::getTel() const
{
	return tel;
}

void CommEntry::setTel(const std::string newTel)
{
	if (newTel != tel) {
		this->tel = newTel;
		std::cout << this->name << "的新电话号码被成功修改为：" << newTel << "！";
	}
	else {
		std::cout << this->name << "的新电话号码和旧的一样，未发生修改。" << std::endl;
	}
}

//========================================
//这是FriendEntry的函数定义
//========================================
FriendEntry::FriendEntry()
{
}

FriendEntry::~FriendEntry()
{
}

void FriendEntry::input(std::istream& is)
{
	CommEntry::input(is);
	if (is.rdbuf() == std::cin.rdbuf()) std::cout << "你正在输入电子邮箱：" << std::endl;
	is >> email;
}

void FriendEntry::output(std::ostream& os) const
{
	os << std::setw(15) << this->getName() << std::setw(15) << this->getTel() << std::setw(20) << this->getEmail() << std::endl;
}

std::string FriendEntry::getEmail() const
{
	return email;
}

AddressBook::AddressBook(const int max) :maxCount(max), currentCount(0), pCes(nullptr)
{
}

AddressBook::~AddressBook()
{
	if (pCes != nullptr) {
		delete[] pCes;
	}
}

void AddressBook::addEntry(int type)
{
	try {
		CommEntry** new_pCes = new CommEntry * [currentCount + 1];

		for (int i = 0; i < currentCount; ++i) {
			new_pCes[i] = pCes[i];
		}

		switch (type)
		{
			//1=一般条目
		case 1:
			new_pCes[currentCount] = new CommEntry;
			break;

			//2=朋友条目
		case 2:
			new_pCes[currentCount] = new FriendEntry;
			break;

		default:
			std::cout << "插入了无效的条目类型！请输入1或2。" << std::endl;
			delete[] new_pCes;
			return;
		}
		delete[] pCes;
		pCes = new_pCes;
		++currentCount;
	}

	catch (std::bad_alloc& e) {
		std::cerr << "内存分配失败: " << e.what() << std::endl;
		throw;
	}
}

void AddressBook::displayAll()
{
	std::cout << "\n=== 显示所有条目 ===" << std::endl;
	std::cout << "通讯录名单如下：" << std::endl
		<< "--------------------------------------------------" << std::endl;
	std::cout << std::setw(15) << "姓名" << std::setw(15) << "电话号码" << std::setw(20) << "电子邮件地址" << std::endl;
	for (int i = 0; i < currentCount; ++i) {
		pCes[i]->output();
	}
	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "总共 " << currentCount << "人" << std::endl << std::endl;
}

int AddressBook::findByName(const std::string& name)
{
	for (int i = 0; i < currentCount; ++i) {
		if (pCes[i]->getName() == name) {
			return i;
		}
	}
	return -1;
}

bool AddressBook::modifyTel(const std::string& name, const std::string& newTel)
{
	int index = findByName(name);
	if (index != -1) {
		pCes[index]->setTel(newTel);
		return true;
	}
	else return false;
}

void AddressBook::saveToFile(const std::string& filename)
{
	try {
		std::cout << "正在保存电话本……" << std::endl;
		std::fstream fout;
		fout.open(filename, std::ios::out);
		fout << std::setw(15) << "姓名" << std::setw(15) << "电话" << std::setw(20) << "邮箱" << std::endl;
		for (int i = 0; i < currentCount; ++i) {
			pCes[i]->output(fout);
		}
		std::cout << "保存成功！" << std::endl;
	}
	catch (std::ios_base::failure& e) {
		std::cerr << "文件操作失败: " << e.what() << std::endl;
		throw;
	}
}

void AddressBook::loadFromFile(const std::string& filename)
{
	std::cout << "电话本的格式应该为：姓名一列，电话号一列，邮箱一列（如果有），并且有标题行" << std::endl;

	try {
		std::cout << "正在尝试读取 " << filename << " 电话本……" << std::endl;
		std::fstream fin;
		fin.open(filename, std::ios::in);

		//跳过标题行
		std::string header;
		std::getline(fin, header);

		std::vector<CommEntry*> tempEntries;

		std::string line;
		while (std::getline(fin, line)) {
			if (line.empty()) continue;

			std::istringstream iss(line);
			std::string name, tel, email;
			iss >> name >> tel >> email;

			CommEntry* entry = nullptr;
			if (!email.empty()) {
				entry = new FriendEntry;
			}
			else {
				entry = new CommEntry;
			}

			std::istringstream dataStream(name + " " + tel + (email.empty() ? "" : " " + email));
			if (entry) {
				entry->input(dataStream);
				tempEntries.push_back(entry);
			}
		}

		if (pCes) {
			for (int i = 0; i < currentCount; ++i) delete pCes[i];
			delete[] pCes;
		}

		currentCount = static_cast<int>(tempEntries.size());
		if (currentCount > 0) {
			pCes = new CommEntry * [currentCount];
			for (int i = 0; i < currentCount; ++i) {
				pCes[i] = tempEntries[i];
			}
		}
		else {
			pCes = nullptr;
		}

		std::cout << "读取成功！共" << currentCount << "条记录" << std::endl;
	}
	catch (std::ios_base::failure& e) {
		std::cerr << "文件操作失败: " << e.what() << std::endl;
		throw;
	}
}

void AddressBook::showSeparate()
{
	std::vector<CommEntry*> normalEntries;
	std::vector<FriendEntry*> friendEntries;

	for (int i = 0; i < currentCount; ++i) {
		FriendEntry* pf = dynamic_cast<FriendEntry*>(pCes[i]);
		if (pf) {
			friendEntries.push_back(pf);
		}
		else {
			normalEntries.push_back(pCes[i]);
		}
	}

	// 显示朋友条目
	std::cout << "\n==================== 朋友条目 ====================" << std::endl;
	if (friendEntries.empty()) {
		std::cout << "(无)" << std::endl;
	}
	else {
		std::cout << std::setw(15) << "姓名" << std::setw(15) << "电话号码"
			<< std::setw(20) << "电子邮件地址" << std::endl;
		for (auto pf : friendEntries) {
			pf->output();
		}
	}

	// 显示一般条目
	std::cout << "==================== 一般条目 ====================" << std::endl;
	if (normalEntries.empty()) {
		std::cout << "(无)" << std::endl;
	}
	else {
		std::cout << std::setw(15) << "姓名" << std::setw(15) << "电话号码" << std::endl;
		for (auto pn : normalEntries) {
			pn->output();
		}
	}
	std::cout << std::endl;
}