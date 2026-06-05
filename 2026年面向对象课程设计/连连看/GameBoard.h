#pragma once
// ============================================================================
// 文件：GameBoard.h
// 角色：连连看游戏的"大脑"——负责所有棋盘逻辑和路径查找算法
//
// 这个类不涉及任何界面绘制（Qt 信号槽都用不到），是一个"纯 C++ 类"。
// 为什么要分开？这就是面向对象的核心思想——"各司其职"：
//   - GameBoard 只管数据（棋盘上有什么）和算法（怎么连）
//   - GameWidget 只管显示（画出来）和交互（鼠标点击）
//   - MainWindow 只管窗口框架（菜单、状态栏、计时）
// 这样修改算法时不用动界面代码，改界面时不用碰算法，互不干扰。
//
// 本次更新：
//   1. 棋盘维度动态化（支持简单/中等/困难三种难度）
//   2. 加入连击分系统（短时间内连续消除获得额外分数）
//   3. 加入序列化方法（支持保存/恢复棋盘状态）
// ============================================================================

#include <QVector>    // QVector —— Qt 动态数组，用于存储棋盘数据和收集方块位置
#include <QPoint>     // QPoint —— 二维整数坐标 (x=列, y=行)，用于表示方块的行列位置
#include <QDateTime>  // QDateTime —— 日期时间类，用 currentMSecsSinceEpoch() 做连击窗口的时间戳判断
#include <random>     // std::mt19937 —— Mersenne Twister 高质量随机数引擎，生成随机棋盘布局
#include <algorithm>  // std::shuffle —— 标准库洗牌算法，用于打乱方块顺序

// ============================================================================
// PathInfo —— 路径信息结构体
//
// 这个结构体的作用是：描述两个方块之间的"连接路径"。
// 连连看的规则是：找到两个相同图案的方块，用不超过两次转弯的路径连起来。
// PathInfo 就记录了这条路径经过的所有"拐弯点"。
// ============================================================================
struct PathInfo
{
    bool valid = false;            // 是否找到有效路径？
    QVector<QPoint> corners;       // 路径上的所有拐点（包含起点和终点）
};

// ============================================================================
// GameBoard —— 游戏棋盘逻辑类
//
// 这个类管理一个"带边界的棋盘"。
// 为什么需要边界？因为连连看允许通过棋盘外部的空白区域连接！
//
// 棋盘布局示意（以标准 8×10 为例，加上边界变成 10×12）：
//
//     列:  0   1   2   ...  10  11
//   行0:  [空][空][空] ... [空][空]   ← 上边界
//   行1:  [空][🐱][🐶] ... [🐰][空]   ← 实际游戏第1行
//   ...
//   行8:  [空][🐔][🦊] ... [🐼][空]   ← 实际游戏第8行
//   行9:  [空][空][空] ... [空][空]   ← 下边界
//
// TOTAL_ROWS = ROWS + 2, TOTAL_COLS = COLS + 2
// ============================================================================
class GameBoard
{
public:
    // ======================== 常量 ========================
    // 注意：ROWS/COLS 现在是构造时动态设置的，但保留默认值以便兼容

    static constexpr int DEFAULT_ROWS = 8;
    static constexpr int DEFAULT_COLS = 10;
    static constexpr int DEFAULT_TILE_TYPES = 20;
    static constexpr int DEFAULT_COPIES = 4;

    static constexpr int EMPTY = 0;

    // 连击窗口：多久内连续消除算连击（毫秒）
    static constexpr int COMBO_WINDOW_MS = 5000;

    // ======================== 构造与初始化 ========================

    // 构造函数
    // rows=游戏行数, cols=游戏列数, tileTypes=图案种类, copiesPerType=每种图案副本数
    GameBoard(int rows = DEFAULT_ROWS, int cols = DEFAULT_COLS,
              int tileTypes = DEFAULT_TILE_TYPES,
              int copiesPerType = DEFAULT_COPIES);

    void initBoard();   // 生成新棋盘（随机放置方块）
    void reset();       // 重置游戏（等效于 initBoard）

    // ======================== 路径查找（核心算法！）========================

    PathInfo findPath(int r1, int c1, int r2, int c2) const;

    // ======================== 游戏状态查询 ========================

    bool hasValidMoves() const;
    PathInfo findHint() const;
    bool isWin() const;

    // ======================== 游戏操作 ========================

    void removeTiles(int r1, int c1, int r2, int c2);
    void shuffle();

    // ======================== 连击系统 ========================

    // 计算本次消除应得分数（含连击加成）
    // 必须在 removeTiles 之前调用，因为 removeTiles 会记录消除时间
    int calculateComboScore();

    // 重置连击（超时归零）
    void resetCombo() { m_comboCount = 0; }
    int getComboCount() const { return m_comboCount; }

    // ======================== 访问器 ========================

    int getTile(int r, int c) const { return m_grid[r][c]; }
    void setTile(int r, int c, int type) { m_grid[r][c] = type; }
    bool isEmpty(int r, int c) const { return m_grid[r][c] == EMPTY; }
    int getScore() const { return m_score; }
    int getRemainingTiles() const { return m_remainingTiles; }
    int getMoves() const { return m_moves; }
    void addScore(int points) { m_score += points; }

    // 棋盘维度
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    int totalRows() const { return m_rows + 2; }
    int totalCols() const { return m_cols + 2; }
    int tileTypes() const { return m_tileTypes; }
    int copiesPerType() const { return m_copiesPerType; }
    int totalTiles() const { return m_rows * m_cols; }

    // ======================== 序列化（用于存档） ========================

    // 把整个棋盘数据导出为一维整数数组
    QVector<int> serializeGrid() const;
    // 从一维整数数组恢复棋盘（不包含分数/步数/连击，那些由上层管理）
    void deserializeGrid(const QVector<int>& data);

    // 手动设置状态（用于从存档恢复）
    void setScore(int s) { m_score = s; }
    void setMoves(int m) { m_moves = m; }
    void setRemainingTiles(int r) { m_remainingTiles = r; }
    void setComboCount(int c) { m_comboCount = c; }

private:
    // ======================== 数据成员 ========================

    // 棋盘维度（构造时设定，之后不变）
    int m_rows;           // 实际游戏行数
    int m_cols;           // 实际游戏列数
    int m_tileTypes;      // 图案种类数
    int m_copiesPerType;  // 每种图案的副本数

    // 棋盘数据：动态二维数组——因为维度在运行时决定，不能用静态数组
    QVector<QVector<int>> m_grid;

    int m_score;
    int m_remainingTiles;
    int m_moves;

    // ---- 连击系统 ----
    int m_comboCount;              // 当前连击数（连续消除对数）
    qint64 m_lastMatchTimeMs;      // 上一次消除的时间戳（毫秒）

    // ======================== 路径查找辅助函数 ========================

    bool isRowClear(int r, int c1, int c2) const;
    bool isColClear(int r1, int r2, int c) const;
    PathInfo tryDirectLink(int r1, int c1, int r2, int c2) const;
    PathInfo tryOneTurnLink(int r1, int c1, int r2, int c2) const;
    PathInfo tryTwoTurnLink(int r1, int c1, int r2, int c2) const;

    // ======================== 随机数 ========================

    std::mt19937 m_rng;
};
