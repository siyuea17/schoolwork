#pragma once

#include <QVector>
#include <QPoint>
#include <random>
#include <algorithm>

// 路径信息结构体：存储连接路径的拐点
struct PathInfo
{
    bool valid = false;                  // 是否找到有效路径
    QVector<QPoint> corners;             // 路径拐点（包含起点和终点）
    // 0 折: 2个点 [起点, 终点]
    // 1 折: 3个点 [起点, 拐角, 终点]
    // 2 折: 4个点 [起点, 拐角1, 拐角2, 终点]
};

// 游戏棋盘逻辑类（纯C++，无Q_OBJECT）
class GameBoard
{
public:
    // 游戏区域尺寸
    static constexpr int ROWS = 8;           // 实际游戏行数
    static constexpr int COLS = 10;          // 实际游戏列数
    static constexpr int TOTAL_ROWS = ROWS + 2;   // 含边界（上下各1行空行）
    static constexpr int TOTAL_COLS = COLS + 2;   // 含边界（左右各1列空列）
    static constexpr int TILE_TYPES = 20;    // 不同方块类型数量
    static constexpr int EMPTY = 0;          // 空格标记
    static constexpr int TOTAL_TILES = ROWS * COLS; // 总方块数 80

    GameBoard();

    // 初始化与重置
    void initBoard();                        // 生成随机棋盘
    void reset();                            // 重置游戏

    // 路径查找（核心算法）
    PathInfo findPath(int r1, int c1, int r2, int c2) const;

    // 游戏状态
    bool hasValidMoves() const;              // 是否还有可消除的配对
    PathInfo findHint() const;               // 找一个可消除的配对
    bool isWin() const;                      // 是否已通关

    // 操作
    void removeTiles(int r1, int c1, int r2, int c2);  // 移除一对方块
    void shuffle();                          // 重排剩余方块

    // 访问器
    int getTile(int r, int c) const { return m_grid[r][c]; }
    void setTile(int r, int c, int type) { m_grid[r][c] = type; }
    bool isEmpty(int r, int c) const { return m_grid[r][c] == EMPTY; }
    int getScore() const { return m_score; }
    int getRemainingTiles() const { return m_remainingTiles; }
    int getMoves() const { return m_moves; }
    void addScore(int points) { m_score += points; }

private:
    int m_grid[TOTAL_ROWS][TOTAL_COLS];      // 棋盘数组
    int m_score;                              // 当前分数
    int m_remainingTiles;                     // 剩余方块数
    int m_moves;                              // 已走步数

    // 路径查找辅助函数
    bool isRowClear(int r, int c1, int c2) const;    // 同一行两点间是否无阻挡
    bool isColClear(int r1, int r2, int c) const;    // 同一列两点间是否无阻挡

    // 各折数连接尝试
    PathInfo tryDirectLink(int r1, int c1, int r2, int c2) const;
    PathInfo tryOneTurnLink(int r1, int c1, int r2, int c2) const;
    PathInfo tryTwoTurnLink(int r1, int c1, int r2, int c2) const;

    // 随机数生成器
    std::mt19937 m_rng;
};
