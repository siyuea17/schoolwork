/****************************************************************/
/*  程  序:	 学生成绩管理系统           	             		*/
/*  功  能:  主菜单的选择										*/
/*  作  者:  叶亚琴                        						*/
/*  时  间:  2025-12-11                         				*/
/*  修  改:  王新疆												*/
/*  时  间:  2025-12-17                           				*/
/****************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include <iomanip>
#include<cstdio>
#include<cstdlib>
#include <limits>
#ifdef _WIN32
#include<conio.h>
#else
#endif

using namespace std;

#define MAX_SIZE 30				//学生成绩单最大长度

int main(void)
{
	int stuListSize;			//学生成绩单实际长度,初始情况下长度为0
	char number[MAX_SIZE][10];	//学号
	char name[MAX_SIZE][11];	//姓名
	float score[MAX_SIZE][4];	//各科成绩，按照数组第二维下标，分别对应数学、语文、英语、平均成绩
	int statistics[4][5];		//分段统计结果，分数段分别是[100,90],(90,80],(80,70],(70,60],(60,0]
	char subject[4][5] = { "数学","语文","英语","平均" };	//科目名称
	//统计成绩模块使用
	char choice;   //表示编号	
	int n, i, j;
	stuListSize = 0;				//创建空表

	char choice2;
	float temp1, temp2, temp3;
	long flag = -1;

	char temp[10];
	do
	{
		system("cls");
		cout << "      *------------------------------------------------------------*\n";
		cout << "      |              中国地质大学（武汉）学生成绩管理系统          |\n";
		cout << "      |                  请输入选项编号（0～8）:                   |\n";
		cout << "      |------------------------------------------------------------|\n";
		cout << "      |                                                            |\n";
		cout << "      |                    1 —— 创建成绩单                         |\n";
		cout << "      |                    2 —— 添加学生                           |\n";
		cout << "      |                    3 —— 编辑学生                           |\n";
		cout << "      |                    4 —— 查找学生                           |\n";
		cout << "      |                    5 —— 浏览成绩单                         |\n";
		cout << "      |                    6 —— 排序成绩单                         |\n";
		cout << "      |                    7 —— 统计成绩                           |\n";
		cout << "      |                    0 —— 退    出                           |\n";
		cout << "      |------------------------------------------------------------|\n";
		cout << "      |         开发者：19C23B-王新疆-20251003860 @2025		   |\n";
		cout << "      *------------------------------------------------------------*\n";
		cout << "请同学们自己完成：3,4,7三个功能。\n";
		choice = _getch();		//接收选项
		switch (choice)		//实现点菜
		{
		case '1':                   //创建成绩单
			if (stuListSize > 0)	//非空表
			{
				cout << "\n不能重新创建学生成绩单！\n";
			}
			else				//空表
			{
				cout << "请输入学生人数：";		//输入学生人数
				cin >> n;
				if (n > 0 && n <= MAX_SIZE)	//学生人数合法，[1,MAX_SIZE]
				{
					for (i = 0; i < n; i++)	//输入n个学生
					{
						cin.clear();
						cin.ignore(numeric_limits<streamsize>::max(), '\n');					//清空键盘缓冲区
						cout << "请输入第" << i + 1 << "条记录\n";
						cout << "请输入学号：";				//输入学号
						cin >> number[i];
						cout << "请输入姓名：";				//输入姓名
						cin >> name[i];
						cout << "请输入数学成绩：";			//输入数学成绩
						cin >> score[i][0];
						cout << "请输入语文成绩：";			//输入语文成绩
						cin >> score[i][1];
						cout << "请输入英语成绩：";			//输入英语成绩
						cin >> score[i][2];
						score[i][3] = (score[i][0] + score[i][1] + score[i][2]) / 3;	//计算平均成绩
					}
					stuListSize = n;	//设置当前表长为n;
					cout << "创建" << stuListSize << "条学生记录成功！\n";
				}
				else	//学生人数不合法
				{
					cout << "学生人数范围应在[1," << MAX_SIZE << "]之间，创建学生成绩单失败！\n";
				}
			}
			break;
		case '2':
			cout << "您选择了\"2 —— 添加学生\"\n";
			if (stuListSize == MAX_SIZE)
			{
				cout << "学生人数范围应在[1," << MAX_SIZE << "]之间，添加学生失败！\n";
			}
			else if (stuListSize == 0)
			{
				cout << "成绩单为空，请先创建成绩单！\n";
			}
			else
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');					//清空键盘缓冲区
				cout << "请输入第" << stuListSize + 1 << "条记录\n";
				cout << "请输入学号：";				//输入学号
				cin >> number[stuListSize];
				for (i = 0; i < stuListSize; i++) //学号不能重复 
				{
					if (strcmp(number[i], number[stuListSize]) == 0)
					{
						cout << "输入学号重复！请重新输入！";
						break;
					}
				}
				if (i == stuListSize)
				{

					cout << "请输入姓名：";				//输入姓名
					cin >> name[stuListSize];
					cout << "请输入数学成绩：";			//输入数学成绩
					cin >> score[stuListSize][0];
					cout << "请输入语文成绩：";			//输入语文成绩
					cin >> score[stuListSize][1];
					cout << "请输入英语成绩：";			//输入英语成绩
					cin >> score[stuListSize][2];
					score[stuListSize][3] = (score[stuListSize][0] + score[stuListSize][1] + score[stuListSize][2]) / 3;	//计算平均成绩
					stuListSize += 1;
					cout << "创建" << stuListSize << "条学生记录成功！\n";
				}
			}
			break;
		case '3'://请完成（可以编辑每一个学生的信息）
			cout << "您选择了\"3 —— 编辑学生\"\n";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');					//清空键盘缓冲区
			cout << "请输入学号：\n";
			cin >> temp;
			for (i = 0; i < stuListSize; i++)
			{
				if (strcmp(number[i],temp) == 0)
				{
					char choice1;
					int grade;
					cout << "输入1-修改学号，\n输入2-修改姓名，\n输入3-修改数学成绩，\n输入4-修改语文成绩，\n输入5-修改英语成绩，\n输入6-删除该记录。\n";
					choice1 = _getch();
					switch (choice1)
					{
					case '1':
						cout << "您选择编辑学号\n";
						cout << "请输入学号：";
						char newNum[10];
						cin >> newNum;
						bool duplicate;
						duplicate = false;
						for (int k = 0; k < stuListSize; k++) {
							if (k != i && strcmp(number[k], newNum) == 0) {
								duplicate = true;
								break;
							}
						}
						if (duplicate) {
							cout << "学号重复，修改失败！\n";
						}
						else {
							strcpy(number[i], newNum);
							cout << "修改完成！\n";
						}
						break;
					case '2':
						cout << "您选择修改姓名\n";
						cout << "请输入姓名\n";
						cin >> name[i];
						cout << "修改完成！\n";
						break;
					case '3':
						cout << "您选择修改数学成绩\n";
						cout << "请输入数学成绩\n";
						cin >> score[i][0];
						score[i][3] = (score[i][0] + score[i][1] + score[i][2]) / 3;
						cout << "修改完成！\n";
						break;
					case '4':
						cout << "您选择修改语文成绩\n";
						cout << "请输入语文成绩\n";
						cin >> score[i][1];
						score[i][3] = (score[i][0] + score[i][1] + score[i][2]) / 3;
						cout << "修改完成！\n";
						break;
					case '5':
						cout << "您选择修改英语成绩\n";
						cout << "请输入英语成绩\n";
						cin >> score[i][2];
						score[i][3] = (score[i][0] + score[i][1] + score[i][2]) / 3;
						cout << "修改完成！\n";
						break;
					case '6':
						cout << "您选择删除该记录\n";
						cout << "您确定吗？（y/n）\n";
						char temp_choice;
						cin >> temp_choice;
						if (temp_choice == 'y' || temp_choice == 'Y') {
							// 将后面的记录前移，覆盖当前记录
							for (int k = i; k < stuListSize - 1; k++) {
								strcpy(number[k], number[k + 1]);
								strcpy(name[k], name[k + 1]);
								for (int j = 0; j < 4; j++) {
									score[k][j] = score[k + 1][j];
								}
							}
							stuListSize -= 1;
							cout << "已经删除！\n";
						stuListSize -= 1;
						cout << "已经删除！\n";
						break;
						}
						else {
							cout << "无事发生……\n按任意键继续\n";
							system("pause");
							break;
						}
					default:
						cout << "非法输入\n";
						break;
					}
				}
				if (i == stuListSize)
					cout << "查找不到该学生！";
			}
			break;
		case '4':
			cout << "您选择了\"4 —— 查找学生\"\n";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');					//清空键盘缓冲区
			cout << "请输入学号：";
			cin >> temp;
			for (i = 0; i < stuListSize; i++) {
				if (strcmp(number[i], temp) == 0) {
					cout << "该学生的姓名是：" << name[i] << endl;
					cout << "该学生的成绩如下\n";
					cout << "数学：" << score[i][0] << endl;
					cout << "语文：" << score[i][1] << endl;
					cout << "英语：" << score[i][2] << endl;
					cout << "平均：" << score[i][3] << endl;
					break;
				}
				else {
					cout << "未找到该学生！\n";
				}
				cout << "按任意键继续……\n";
				system("pause");
			}
			break;
		case '5':    //浏览成绩单
			if (0 == stuListSize)	//表空
			{
				cout << "无学生记录，请创建成绩单或添加学生！\n";
			}
			else	//表不空
			{
				//输出表头
				cout << setw(4) << "序号"
					<< setw(12) << "学号"
					<< setw(12) << "姓名"
					<< setw(10) << "数学"
					<< setw(10) << "语文"
					<< setw(10) << "英语"
					<< setw(10) << "平均" << endl;

				for (i = 1; i <= 68; i++)
					putchar('=');
				cout << "\n";
				//输出表体
				for (i = 0; i < stuListSize; i++)	//输出stuListSize个学生成绩信息
				{
					cout << setw(4) << i + 1 << setw(12) << number[i] << setw(12) << name[i];  //输出学号、姓名
					for (j = 0; j < 4; j++)		//输出数学、语文、英语、平均成绩
						cout << setw(10) << fixed << setprecision(1) << score[i][j];
					cout << "\n";
				}
				cout << "按任意键继续……\n";
				system("pause");
			}
			break;
		case '6': //排序成绩单
			cout << "您选择了\"6 —— 排序成绩单\"\n";
			cout << "输入0-排序数学成绩，\n输入1-排序语文成绩，\n输入2-排序英语成绩，\n输入3-排序平均分";
			choice2 = _getch();
			switch (choice2)
			{
			case '0':flag = 0; break;
			case '1':flag = 1; break;
			case '2':flag = 2; break;
			case '3':flag = 3; break;
			default:cout << "非法输入！\n";
			}
			bool ordered;
			for (i = 0; i < stuListSize - 1; i++) {
				ordered = true;
				for (j = 0; j < stuListSize - i - 1; j++) {
					if (score[j][flag] < score[j + 1][flag]) {
						char tempNum[10];
						strcpy_s(tempNum, number[j]);
						strcpy_s(number[j], number[j + 1]);
						strcpy_s(number[j + 1], tempNum);

						char tempName[11];
						strcpy_s(tempName, name[j]);
						strcpy_s(name[j], name[j + 1]);
						strcpy_s(name[j + 1], tempName);

						float tempScore[4];
						for (int k = 0; k < 4; k++) {
							tempScore[k] = score[j][k];
							score[j][k] = score[j + 1][k];
							score[j + 1][k] = tempScore[k];
						}
						ordered = false;  
					}
				}
				if (ordered) {
					break;
				}
			}

			break;
		case '7':	//统计成绩
			if (0 == stuListSize)	//表空	
			{
				cout << "无学生记录，请创建成绩单！\n";
				break;				//结束switch
			}
			memset(statistics, 0, sizeof(statistics));  //初始化statistics数组元素都为0
			//开始统计
			if (stuListSize > 0) {
				for (i = 0; i < stuListSize; i++) {//第i个学生
					for (j = 0; j < 4; j++) {//成绩从数学、语文、英语、平均开始遍历
						if (score[i][j] >= 90) {
							statistics[j][0] += 1;
						}
						else if (score[i][j] >= 80) {
							statistics[j][1] += 1;
						}
						else if (score[i][j] >= 70) {
							statistics[j][2] += 1;
						}
						else if (score[i][j] >= 60) {
							statistics[j][3] += 1;
						}
						else {
							statistics[j][4] += 1;
						}
					}
				}
				cout << "统计完成\n";
			}
			//显示统计结果
			cout << setw(10) << "科目" << setw(10) << "90分以上" << setw(10) << "80～89" << setw(10) << "70～79" << setw(10) << "60～69" << setw(10) << "60分以下" << endl;//打印表头
			for (i = 0; i < 4; i++)	//打印四行数据(4科)，每行包括：科目名称，5个分数段人次数
			{
				cout << setw(10) << subject[i];
				for (j = 0; j < 5; j++)
					cout << setw(10) << statistics[i][j];
				cout << "\n";
			}
			cout << "按任意键继续……\n";
			system("pause");
			break;
		case '0':
			cout << "您选择了\"0 —— 退出\"\n";
			break;
		default:
			cout << "非法输入\n";
			break;
		}
	} while (choice != '0');   //当选择0编号时退出系统
	return 0;
}
