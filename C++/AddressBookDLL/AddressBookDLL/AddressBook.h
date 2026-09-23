#pragma once

#ifdef ADDRESSBOOKDLL_EXPORTS
#define AB_API __declspec(dllexport)
#else
#define AB_API __declspec(dllimport)
#endif

#include <string>
#include <iostream>

class AB_API CommEntry
{
protected:
	//姓名
	std::string name;

	//电话号码
	std::string tel;
public:
	CommEntry();
	virtual ~CommEntry();

	//输入姓名和电话
	virtual void input(std::istream& is = std::cin);

	//输出条目信息（多态基函数）
	virtual void output(std::ostream& os = std::cout) const;

	//getter
	std::string getName() const;
	std::string getTel() const;

	//setter
	void setTel(const std::string newTel);
};

class AB_API FriendEntry :
	public CommEntry
{
	// 电子邮件地址
	std::string email;
public:
	FriendEntry();
	virtual ~FriendEntry();

	//输入姓名、电话和邮箱（调用基类input后额外输入email）
	void input(std::istream& is = std::cin);

	//输出格式：姓名、电话、邮箱
	virtual void output(std::ostream& os = std::cout) const override;

	//getter
	std::string getEmail() const;
};

class AB_API AddressBook
{
	//指针数组，存储 CommEntry* 或 FriendEntry*
	CommEntry** pCes;

	//最大容量（由用户输入决定）
	int maxCount;

	//当前实际条目数量
	int currentCount;

public:
	AddressBook(const int max);
	virtual ~AddressBook();

	//根据type(1=一般条目,2=朋友条目)创建对应对象并输入数据
	void addEntry(int type);

	//多态调用output()显示所有条目
	void displayAll();

	//返回索引，未找到返回-1
	int findByName(const std::string& name);

	//修改电话
	bool modifyTel(const std::string& name, const std::string& newTel);

	//保存所有条目到文件
	void saveToFile(const std::string& filename);

	//从文件读取条目（覆盖现有数据）
	void loadFromFile(const std::string& filename);

	//（选做）利用RTTI分开显示朋友条目与一般条目
	void showSeparate();
};

