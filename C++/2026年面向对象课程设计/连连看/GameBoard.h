#pragma once
// GameBoard.h — 连连看棋盘逻辑与路径查找

#include <QVector>
#include <QPoint>
#include <QDateTime>
#include <random>
#include <algorithm>

// PathInfo —— 路径信息：记录两个方块之间的连接路径（拐点序列）
struct PathInfo
{
    bool valid = false;
    QVector<QPoint> corners;  // 路径拐点（含起点和终点）
};

class GameBoard
{
public:
    // ---- 默认参数 ----
    static constexpr int DEFAULT_ROWS = 8;
    static constexpr int DEFAULT_COLS = 10;
    static constexpr int DEFAULT_TILE_TYPES = 20;
    static constexpr int DEFAULT_COPIES = 4;
    static constexpr int EMPTY = 0;
    static constexpr int COMBO_WINDOW_MS = 5000;  // 连击窗口（毫秒）

    // ---- 构造与初始化 ----
    GameBoard(int rows = DEFAULT_ROWS, int cols = DEFAULT_COLS,
              int tileTypes = DEFAULT_TILE_TYPES,
              int copiesPerType = DEFAULT_COPIES);
    void initBoard();
    void reset();

    // ---- 路径查找（0-转弯/1-转弯/2-转弯算法） ----
    PathInfo findPath(int r1, int c1, int r2, int c2) const;

    // ---- 游戏状态查询 ----
    bool hasValidMoves() const;
    PathInfo findHint() const;
    bool isWin() const;

    // ---- 游戏操作 ----
    void removeTiles(int r1, int c1, int r2, int c2);
    void shuffle();

    // ---- 连击系统 ----
    int calculateComboScore();      // 计算含连击加成的分数（须在 removeTiles 前调用）
    void resetCombo() { m_comboCount = 0; }
    int getComboCount() const { return m_comboCount; }

    // ---- 访问器 ----
    int getTile(int r, int c) const { return m_grid[r][c]; }
    void setTile(int r, int c, int type) { m_grid[r][c] = type; }
    bool isEmpty(int r, int c) const { return m_grid[r][c] == EMPTY; }
    int getScore() const { return m_score; }
    int getRemainingTiles() const { return m_remainingTiles; }
    int getMoves() const { return m_moves; }
    void addScore(int points) { m_score += points; }
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    int totalRows() const { return m_rows + 2; }
    int totalCols() const { return m_cols + 2; }
    int tileTypes() const { return m_tileTypes; }
    int copiesPerType() const { return m_copiesPerType; }
    int totalTiles() const { return m_rows * m_cols; }

    // ---- 序列化（存档用） ----
    QVector<int> serializeGrid() const;
    void deserializeGrid(const QVector<int>& data);
    void setScore(int s) { m_score = s; }
    void setMoves(int m) { m_moves = m; }
    void setRemainingTiles(int r) { m_remainingTiles = r; }
    void setComboCount(int c) { m_comboCount = c; }

private:
    // ---- 数据成员 ----
    int m_rows, m_cols;
    int m_tileTypes, m_copiesPerType;
    QVector<QVector<int>> m_grid;  // 动态二维数组（带边界）
    int m_score = 0;
    int m_remainingTiles = 0;
    int m_moves = 0;
    int m_comboCount = 0;
    qint64 m_lastMatchTimeMs = 0;

    // ---- 路径查找辅助 ----
    // 0-转弯：同行或同列且路径完全通畅
    // 1-转弯：在拐角处转弯一次，行列分别通畅
    // 2-转弯：两条平行线通过中间桥接列/行连接
    bool isRowClear(int r, int c1, int c2) const;
    bool isColClear(int r1, int r2, int c) const;
    PathInfo tryDirectLink(int r1, int c1, int r2, int c2) const;
    PathInfo tryOneTurnLink(int r1, int c1, int r2, int c2) const;
    PathInfo tryTwoTurnLink(int r1, int c1, int r2, int c2) const;

    std::mt19937 m_rng;
};
