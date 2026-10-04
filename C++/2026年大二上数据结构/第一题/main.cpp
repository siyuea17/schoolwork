// 题目1：线性表的应用 —— 程序入口
//
// 运行后先显示控制台菜单，用键盘输入选择：
//   1) 图形界面（Qt Widgets）：按钮/输入框操作两个线性表
//   2) 控制台演示：键盘输入整数、@ 结束，带菜单循环
//      （满足题面“交互 UI，不能一次运行完”的要求）
//   0) 退出
//
// 本程序没有任何命令行选项，所有选择都在菜单里完成。
// main 的 argc/argv 只是为了转交给 QApplication，不是命令行开关。

#include <cstdlib>
#include <cerrno>
#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>

#include <QApplication>
#include <QFont>
#include <QString>

#include "SeqList.h"
#include "SinglyLinkedList.h"
#include "mainwindow.h"

// 源码统一是 UTF-8（带 BOM），编译时加了 /utf-8，所以字符串字面量在可执行文件里
// 是 UTF-8 字节。Windows 控制台默认用系统 ANSI 代码页（中文系统是 936/GBK）显示，
// 直接输出就会乱码。这里在进入菜单之前把控制台输入输出代码页切到 UTF-8。
namespace
{
	void SetupConsoleUtf8()
	{
		SetConsoleOutputCP(65001);
		SetConsoleCP(65001);
	}

	// 创建 Qt 应用、显示主窗口，进入事件循环
	//
	// QApplication 的构造函数必须拿到 argc/argv（它要按引用修改它们），
	// 所以这里的参数不是“命令行开关”，只是为了满足 Qt 的接口要求。
	int RunGui(int argc, char* argv[])
	{
		QApplication app(argc, argv);

		// Windows 默认字体对中文支持不好，显式指定微软雅黑
		QFont font(QString::fromLocal8Bit("Microsoft YaHei"));
		font.setPointSize(10);
		app.setFont(font);

		MainWindow window;
		window.show();
		return app.exec();
	}

	bool ParseBoundedValue(const std::string& text, int* value)
	{
		char* end = nullptr;
		errno = 0;
		const long parsed = std::strtol(text.c_str(), &end, 10);
		if (end == text.c_str() || *end != '\0' || errno == ERANGE ||
			parsed < INT_MIN || parsed > INT_MAX || parsed >= 100)
		{
			return false;
		}
		*value = static_cast<int>(parsed);
		return true;
	}

	// 持续读取多行，直到遇到 @；只接受小于 100 的整数。
	int ReadIntegersUntilAt(std::vector<int>* values)
	{
		std::cout << "请输入整数（可分多行输入，@ 结束；每个整数必须小于 100）：" << std::endl;
		std::cout << "> ";

		std::string line;
		bool finished = false;
		while (!finished && std::getline(std::cin, line))
		{
			const size_t at = line.find('@');
			std::string part = (at == std::string::npos) ? line : line.substr(0, at);
			for (char& ch : part)
			{
				if (ch == ',')
				{
					ch = ' ';
				}
			}

			std::istringstream stream(part);
			std::string token;
			while (stream >> token)
			{
				int value = 0;
				if (!ParseBoundedValue(token, &value))
				{
					std::cout << "已跳过非法输入（必须是小于 100 的整数）：" << token << std::endl;
					continue;
				}
				values->push_back(value);
			}

			if (at != std::string::npos)
			{
				finished = true;
			}
			else
			{
				std::cout << "> ";
			}
		}
		return static_cast<int>(values->size());
	}

	// 打印两个表的当前内容
	void PrintTables(SeqList<int>& seq, SinglyLinkedList<int>& list)
	{
		std::cout << "顺序表（" << seq.Length() << " 个元素）：";
		if (seq.IsEmpty())
		{
			std::cout << "（空）";
		}
		else
		{
			seq.Output();
			std::cout << "                     ";
		}
		std::cout << std::endl;

		std::cout << "单链表（" << list.Size() << " 个结点）：";
		if (list.IsEmpty())
		{
			std::cout << "（空）";
		}
		else
		{
			list.Output();
			std::cout << "                     ";
		}
		std::cout << std::endl;
	}

	int RunConsoleDemo()
	{
		// 顺序表初始容量，装满后 Insert 会自动翻倍扩容
		SeqList<int> seq(16);
		SinglyLinkedList<int> list;

		std::cout << std::endl;
		std::cout << "========== 线性表演示（控制台版） ==========" << std::endl;
		std::cout << "顺序表 SeqList<int> 与单链表 SinglyLinkedList<int> 都继承自 LinearList<int>" << std::endl;

		std::vector<int> values;

		while (true)
		{
			std::cout << std::endl;
			std::cout << "---------- 主菜单 ----------" << std::endl;
			std::cout << "  1) 表插入（输入任意个整数，@ 结束，插入两个表）" << std::endl;
			std::cout << "  2) 元素删除（按元素值删除，相同值全部删除）" << std::endl;
			std::cout << "  3) 在指定元素值之后插入（多个只算第一个）" << std::endl;
			std::cout << "  4) 单链表原地逆序（不借助辅助数组/链表）" << std::endl;
			std::cout << "  5) 两个表排序（升序）" << std::endl;
			std::cout << "  6) 显示两个表的全部元素" << std::endl;
			std::cout << "  7) 清空两个表" << std::endl;
			std::cout << "  0) 退出系统" << std::endl;
			std::cout << "请选择：";

			std::string choice;
			if (!std::getline(std::cin, choice))
			{
				break;
			}
			if (choice.empty())
			{
				continue;
			}
			const char c = choice[0];
			if (c == '0')
			{
				std::cout << "已退出。" << std::endl;
				break;
			}

			if (c == '1')
			{
				values.clear();
				const int n = ReadIntegersUntilAt(&values);
				if (n == 0)
				{
					std::cout << "（没有读到整数）" << std::endl;
					continue;
				}
				for (int i = 0; i < n; i++)
				{
					const int v = values[i];
					// 顺序表按位序追加；单链表直接沿指针追加，不按下标插入。
					seq.Insert(seq.Length() + 1, v);
					list.Append(v);
				}
				std::cout << "已向两个表各插入 " << n << " 个元素。" << std::endl;
				PrintTables(seq, list);
			}
			else if (c == '2')
			{
				std::cout << "请输入要删除的元素值：";
				std::string s;
				std::getline(std::cin, s);
				if (s.empty())
				{
					continue;
				}
				int v = 0;
				if (!ParseBoundedValue(s, &v))
				{
					std::cout << "请输入小于 100 的整数。" << std::endl;
					continue;
				}
				const int seqBefore = seq.Length();
				const int listBefore = list.Size();
				const bool seqOk = seq.Delete(v);
				const bool listOk = list.Delete(v);
				std::cout << "删除值 " << v << "：顺序表 " << seqBefore << " -> " << seq.Length()
					<< "（" << (seqOk ? "已删除" : "表中没有该值") << "），单链表 "
					<< listBefore << " -> " << list.Size()
					<< "（" << (listOk ? "已删除" : "表中没有该值") << "）" << std::endl;
				PrintTables(seq, list);
			}
			else if (c == '3')
			{
				std::cout << "请输入用于定位的元素值：";
				std::string s1;
				std::getline(std::cin, s1);
				std::cout << "请输入要插入的元素值：";
				std::string s2;
				std::getline(std::cin, s2);
				if (s1.empty() || s2.empty())
				{
					continue;
				}
				int target = 0;
				int newValue = 0;
				if (!ParseBoundedValue(s1, &target) || !ParseBoundedValue(s2, &newValue))
				{
					std::cout << "请输入两个小于 100 的整数。" << std::endl;
					continue;
				}
				const bool seqOk = seq.InsertAfter(target, newValue);
				const bool listOk = list.InsertAfter(target, newValue);
				if (seqOk || listOk)
				{
					std::cout << "已在第一个 " << target << " 之后插入 " << newValue
						<< "：顺序表" << (seqOk ? "成功" : "未找到该元素")
						<< "，单链表" << (listOk ? "成功" : "未找到该元素") << std::endl;
				}
				else
				{
					std::cout << "两个表中都没有值为 " << target << " 的元素，未插入。" << std::endl;
				}
				PrintTables(seq, list);
			}
			else if (c == '4')
			{
				const int before = list.Size();
				list.Reverse();
				std::cout << "单链表已原地反转（结点数 " << before << " 不变，未使用辅助数组/链表）。" << std::endl;
				PrintTables(seq, list);
			}
			else if (c == '5')
			{
				seq.Sort();
				list.Sort();
				std::cout << "两个表均已按元素值升序排列。" << std::endl;
				PrintTables(seq, list);
			}
			else if (c == '6')
			{
				PrintTables(seq, list);
			}
			else if (c == '7')
			{
				seq.Clear();
				list.Clear();
				std::cout << "两个表已清空。" << std::endl;
				PrintTables(seq, list);
			}
			else
			{
				std::cout << "无效的选择，请重新输入。" << std::endl;
			}
		}
		return 0;
	}
}

// argc/argv 只是转交给 QApplication 用的（Qt 要求按引用传进来），
// 本程序没有任何命令行选项：所有选择都在下面的菜单里由键盘输入完成。
int main(int argc, char* argv[])
{
	SetupConsoleUtf8();

	// 启动后先给控制台菜单，选 1 进图形界面，选 2 留在控制台
	while (true)
	{
		std::cout << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << "  题目1：线性表的应用（顺序表 / 单链表）" << std::endl;
		std::cout << "======================================================" << std::endl;
		std::cout << "  1) 图形界面（Qt Widgets）" << std::endl;
		std::cout << "  2) 控制台演示（键盘输入整数，@ 结束，带菜单循环）" << std::endl;
		std::cout << "  0) 退出" << std::endl;
		std::cout << "请选择：";

		std::string choice;
		if (!std::getline(std::cin, choice))
		{
			break;
		}
		if (choice.empty())
		{
			continue;
		}
		const char c = choice[0];
		if (c == '0')
		{
			break;
		}
		if (c == '1')
		{
			return RunGui(argc, argv);
		}
		else if (c == '2')
		{
			RunConsoleDemo();
		}
		else
		{
			std::cout << "无效的选择。" << std::endl;
		}
	}
	return 0;
}
