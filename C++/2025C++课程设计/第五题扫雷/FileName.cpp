#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <conio.h>
#include <windows.h>

using namespace std;


class CGame {
private:
	vector<vector<int>> mineMap;//地雷分布图，-1表示雷，0到8表示周围的地雷数量
	vector<vector<char>> displayMap;//显示给玩家的图
	vector<vector<bool>> visited;//记录是否已经被访问
	int rows, cols;//行和列
	int mineCount;//地雷数量
	int disMineCount;//用于展示的地雷数量
	int openedCount;//已经打开了的格子数量
	int cursorX, cursorY;//光标的位置
	bool gameOver;
	bool gameWin;
	bool firstClick;//是否是第一次点击
	int aroundRow[8] = { -1,-1,-1, 0, 0, 1, 1, 1 };
	int aroundCol[8] = { -1, 0, 1,-1, 1,-1, 0, 1 };

	void generateMines(int x, int y);//生成地雷
	int countMinesAround(int x, int y);//计算各自周围的地雷数量
	void openCell(int x, int y);//打开格子
	void openBlank(int x, int y);//打开空白格子和周围的数字格子
	void markMine(int x, int y);//标记地雷
	void moveCursor(char dirrection);//移动光标
	bool isValid(int x, int y);//坐标是否有效
	void printGameOver();
	void printBoard();
public:
	CGame();
	void printRules();
	void Run();
	void difficulty();
};

CGame::CGame() :rows(8), cols(8), mineCount(10),
disMineCount(10), firstClick(true), gameOver(false),
gameWin(false), openedCount(0), cursorX(0), cursorY(0) {
	mineMap.assign(rows, vector<int>(cols, 0));
	displayMap.assign(rows, vector<char>(cols, '#'));
	visited.assign(rows, vector<bool>(cols, false));
}

void CGame::printRules() {
	system("cls");
	cout << "============= 扫雷游戏 =============" << endl;
	cout << "游戏规则：" << endl;
	cout << "1. 这是一个8x8的扫雷游戏，共有10个地雷" << endl;
	cout << "2. 使用wasd移动选择框" << endl;
	cout << "3. 按空格键打开格子" << endl;
	cout << "4. 按'M'键标记地雷" << endl;
	cout << "5. 打开所有非雷格子获胜，踩到地雷失败" << endl;
	cout << "====================================" << endl << endl;
}


void CGame::generateMines(int x, int y) {
	srand(time(0));
	int minesPlaced = 0;

	while (minesPlaced < mineCount) {
		int rx = rand() % rows;
		int ry = rand() % cols;

		if (abs(rx - x) <= 1 && abs(ry - y) <= 1) {
			continue;
		}

		if (mineMap[rx][ry] != -1) {
			mineMap[rx][ry] = -1;
			minesPlaced++;
		}
	}
	//计算每个格子的数字
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (mineMap[i][j] != -1) {
				mineMap[i][j] = countMinesAround(i, j);
			}
		}
	}
}

int CGame::countMinesAround(int x, int y) {
	int count = 0;
	//检查八个方向
	for (int i = 0; i < 8; i++) {
		int nx = x + aroundRow[i];
		int ny = y + aroundCol[i];
		if (isValid(nx, ny) && mineMap[nx][ny] == -1) {
			count++;
		}
	}
	return count;
}

void CGame::openCell(int x, int y) {
	if (displayMap[x][y] == 'M') {
		cout << "这个格子已经被标记，不能打开" << endl;
		return;
	}
	if (firstClick) {
		generateMines(x, y);
		firstClick = false;
	}
	if (mineMap[x][y] == -1) {
		displayMap[x][y] = '*';
		gameOver = true;
		return;
	}

	openBlank(x, y);

	if (openedCount == rows * cols - mineCount) {
		gameWin = true;
	}
}

void CGame::openBlank(int x, int y) {
	if (!isValid(x, y) || visited[x][y] || displayMap[x][y] == 'M') {
		return;
	}

	visited[x][y] = true;
	openedCount++;

	if (mineMap[x][y] > 0) {
		displayMap[x][y] = '0' + mineMap[x][y];
		return;
	}
	else if (mineMap[x][y] == 0) {
		displayMap[x][y] = ' ';

		for (int i = 0; i < 8; i++) {
			int nx = x + aroundRow[i];
			int ny = y + aroundCol[i];
			if (isValid(nx, ny) && !visited[nx][ny] && displayMap[nx][ny] == '#' && displayMap[nx][ny] != 'M') {
				openBlank(nx, ny);
			}
		}
	}
	if (openedCount == rows * cols - mineCount) {
		gameWin = true;
	}
}

void CGame::markMine(int x, int y) {
	if (displayMap[x][y] == '#' && disMineCount > 0 && !firstClick) {
		displayMap[x][y] = 'M';
		disMineCount--;
	}
	else if (displayMap[x][y] == 'M') {
		displayMap[x][y] = '#';
		disMineCount++;
	}
}

bool CGame::isValid(int x, int y) {
	return x >= 0 && x < rows && y >= 0 && y < cols;
}

void CGame::moveCursor(char direction) {
	switch (direction) {
	case'w':case'W'://上
		if (cursorX > 0) {
			cursorX--;
		}
		break;
	case'a':case'A'://左
		if (cursorY > 0) {
			cursorY--;
		}
		break;
	case's':case'S'://下
		if (cursorX < rows - 1) {
			cursorX++;
		}
		break;
	case'd':case'D'://右
		if (cursorY < cols - 1) {
			cursorY++;
		}
		break;
	}
}

void CGame::Run() {
	printRules();
	cout << "按任意键开始游戏" << endl;
	_getch();
	system("cls");
	difficulty();
	printBoard();

	while (!gameOver && !gameWin) {

		char input = _getch();

		switch (input) {
		case'w':case'a':case's':case'd':case'W':case'A':case'S':case'D':
			moveCursor(input);
			printBoard();
			break;
		case' ':
			openCell(cursorX, cursorY);
			printBoard();
			break;
		case'M':case'm':
			markMine(cursorX, cursorY);
			printBoard();
			break;
		case'q':case'Q':
			return;
		default:
			break;
		}
	}
	printGameOver();
}

void CGame::printGameOver() {
	system("cls");
	if (gameWin) {
		cout << "============= 恭喜！ =============" << endl;
		cout << "        你赢得了游戏！" << endl;
	}
	else {
		cout << "============= 游戏结束 =============" << endl;
		cout << "        你踩到了地雷！" << endl << endl;
	}
	//开始修改displayMap,然后用printBoard输出
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (mineMap[i][j] == -1) {
				displayMap[i][j] = '*';
			}
			else if (mineMap[i][j] == 0) {
				displayMap[i][j] = ' ';
			}
			else {
				displayMap[i][j] = mineMap[i][j] + '0';
			}
		}
	}
	//打印棋盘
	cout << "===================================" << endl;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (i == cursorX && j == cursorY) {
				cout << "[" << displayMap[i][j] << "]";
			}
			else {
				cout << " " << displayMap[i][j] << " ";
			}
		}
		cout << endl << endl;
	}
	cout << "===================================" << endl << endl;
}

void CGame::printBoard() {
	system("cls");
	cout << "剩余地雷数量：" << disMineCount << endl;
	cout << "已经打开：" << openedCount << endl;
	cout << "还剩余" << rows * cols - openedCount << "个格子未打开" << endl;
	cout << "------------------------------------" << endl;
	//打印棋盘
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (i == cursorX && j == cursorY) {
				cout << "[" << displayMap[i][j] << "]";
			}
			else {
				cout << " " << displayMap[i][j] << " ";
			}
		}
		cout << endl << endl;
	}
	cout << "-----------------------------------" << endl;
	cout << "操作说明: wasd移动, 空格打开, M标记" << endl;
	cout << "按Q键退出游戏" << endl;
}

void CGame::difficulty() {
	cout << "请选择难度;" << endl;
	cout << "简单：8 X 8，10颗地雷——请按1" << endl;
	cout << "中等：16 X 16，40颗地雷——请按2" << endl;
	cout << "困难：16 X 30，99颗地雷——请按3" << endl;
	cout << "自定义难度：自行输入行数、列数、地雷数量——请按4" << endl;
	cout << "请选择：";
	char choice;
	cin >> choice;
	switch (choice) {
	case'1':
		break;
	case'2':
		rows = 16; cols = 16; mineCount = 40;
		break;
	case'3':
		rows = 16; cols = 30; mineCount = 99;
		break;
	case'4':
		cin >> rows >> cols >> mineCount;
		break;
	default:
		cout << "不是预设的难度，默认使用简单" << endl;
		Sleep(2000);
		break;
	}
	mineMap.assign(rows, vector<int>(cols, 0));
	displayMap.assign(rows, vector<char>(cols, '#'));
	visited.assign(rows, vector<bool>(cols, false));
	cout << "好的！" << rows << "行," << cols << "列，" << "共计" << mineCount << "颗地雷！" << endl;
}

int main() {
	srand(time(0));
	char palyAgain;
	do {
		CGame game;
		game.Run();
		cout << "是否再来一局？（Y/N）：";
		cin >> palyAgain;
	} while (palyAgain != 'N' || palyAgain != 'n');
	return 0;
}