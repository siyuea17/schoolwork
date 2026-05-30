#include "GameBoard.h"
#include <QVector>
#include <algorithm>
#include <random>
#include <chrono>

// ============================================================================
// 构造函数
// ============================================================================
GameBoard::GameBoard()
    : m_score(0)
    , m_remainingTiles(0)
    , m_moves(0)
{
    // 初始化随机数生成器
    m_rng.seed(static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    // 初始清空棋盘
    for (int r = 0; r < TOTAL_ROWS; ++r)
        for (int c = 0; c < TOTAL_COLS; ++c)
            m_grid[r][c] = EMPTY;
}

// ============================================================================
// 初始化棋盘：生成80个方块，每种类型4个，随机放置
// ============================================================================
void GameBoard::initBoard()
{
    // 1. 生成方块列表：每种类型4个
    QVector<int> tiles;
    tiles.reserve(TOTAL_TILES);
    for (int type = 1; type <= TILE_TYPES; ++type)
    {
        for (int i = 0; i < 4; ++i)
            tiles.append(type);
    }

    // 2. 随机打乱
    std::shuffle(tiles.begin(), tiles.end(), m_rng);

    // 3. 放置到游戏区域（跳过边界行/列）
    int index = 0;
    for (int r = 1; r <= ROWS; ++r)
    {
        for (int c = 1; c <= COLS; ++c)
        {
            m_grid[r][c] = tiles[index++];
        }
    }

    // 4. 确保边界全为空
    for (int r = 0; r < TOTAL_ROWS; ++r)
    {
        m_grid[r][0] = EMPTY;
        m_grid[r][TOTAL_COLS - 1] = EMPTY;
    }
    for (int c = 0; c < TOTAL_COLS; ++c)
    {
        m_grid[0][c] = EMPTY;
        m_grid[TOTAL_ROWS - 1][c] = EMPTY;
    }

    m_score = 0;
    m_remainingTiles = TOTAL_TILES;
    m_moves = 0;
}

void GameBoard::reset()
{
    initBoard();
}

// ============================================================================
// 辅助函数：检查同一行/列两点之间是否畅通（不含端点）
// ============================================================================
bool GameBoard::isRowClear(int r, int c1, int c2) const
{
    int minC = (c1 < c2) ? c1 : c2;
    int maxC = (c1 < c2) ? c2 : c1;
    for (int c = minC + 1; c < maxC; ++c)
    {
        if (m_grid[r][c] != EMPTY)
            return false;
    }
    return true;
}

bool GameBoard::isColClear(int r1, int r2, int c) const
{
    int minR = (r1 < r2) ? r1 : r2;
    int maxR = (r1 < r2) ? r2 : r1;
    for (int r = minR + 1; r < maxR; ++r)
    {
        if (m_grid[r][c] != EMPTY)
            return false;
    }
    return true;
}

// ============================================================================
// 0 折连接：同行或同列且之间无阻挡
// ============================================================================
PathInfo GameBoard::tryDirectLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;
    if (r1 == r2 && isRowClear(r1, c1, c2))
    {
        result.valid = true;
        result.corners = { QPoint(c1, r1), QPoint(c2, r2) };
    }
    else if (c1 == c2 && isColClear(r1, r2, c1))
    {
        result.valid = true;
        result.corners = { QPoint(c1, r1), QPoint(c2, r2) };
    }
    return result;
}

// ============================================================================
// 1 折连接（L形）：拐角处必须是空格
// ============================================================================
PathInfo GameBoard::tryOneTurnLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;

    // 拐角1：(r1, c2)
    if (m_grid[r1][c2] == EMPTY)
    {
        if (isRowClear(r1, c1, c2) && isColClear(r1, r2, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1), QPoint(c2, r1), QPoint(c2, r2) };
            return result;
        }
    }

    // 拐角2：(r2, c1)
    if (m_grid[r2][c1] == EMPTY)
    {
        if (isColClear(r1, r2, c1) && isRowClear(r2, c1, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1), QPoint(c1, r2), QPoint(c2, r2) };
            return result;
        }
    }

    return result;
}

// ============================================================================
// 2 折连接（Z/U形）：扫描所有可能的中间行/列
// ============================================================================
PathInfo GameBoard::tryTwoTurnLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;

    // 扫描中间行：垂直线段 → 水平线段 → 垂直线段
    for (int r = 0; r < TOTAL_ROWS; ++r)
    {
        if (r == r1 || r == r2) continue;  // 已由0折或1折处理
        if (m_grid[r][c1] != EMPTY) continue;
        if (m_grid[r][c2] != EMPTY) continue;

        if (isColClear(r1, r, c1) && isRowClear(r, c1, c2) && isColClear(r, r2, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1), QPoint(c1, r), QPoint(c2, r), QPoint(c2, r2) };
            return result;
        }
    }

    // 扫描中间列：水平线段 → 垂直线段 → 水平线段
    for (int c = 0; c < TOTAL_COLS; ++c)
    {
        if (c == c1 || c == c2) continue;  // 已由0折或1折处理
        if (m_grid[r1][c] != EMPTY) continue;
        if (m_grid[r2][c] != EMPTY) continue;

        if (isRowClear(r1, c1, c) && isColClear(r1, r2, c) && isRowClear(r2, c, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1), QPoint(c, r1), QPoint(c, r2), QPoint(c2, r2) };
            return result;
        }
    }

    return result;
}

// ============================================================================
// 主路径查找：依次尝试 0折 → 1折 → 2折
// ============================================================================
PathInfo GameBoard::findPath(int r1, int c1, int r2, int c2) const
{
    // 边界检查
    if (r1 < 0 || r1 >= TOTAL_ROWS || c1 < 0 || c1 >= TOTAL_COLS) return PathInfo();
    if (r2 < 0 || r2 >= TOTAL_ROWS || c2 < 0 || c2 >= TOTAL_COLS) return PathInfo();

    // 必须是同类型非空方块
    if (m_grid[r1][c1] == EMPTY || m_grid[r2][c2] == EMPTY) return PathInfo();
    if (m_grid[r1][c1] != m_grid[r2][c2]) return PathInfo();

    // 同一位置不能匹配
    if (r1 == r2 && c1 == c2) return PathInfo();

    // 0折：直接连接
    PathInfo path = tryDirectLink(r1, c1, r2, c2);
    if (path.valid) return path;

    // 1折：L形连接
    path = tryOneTurnLink(r1, c1, r2, c2);
    if (path.valid) return path;

    // 2折：Z/U形连接
    path = tryTwoTurnLink(r1, c1, r2, c2);
    if (path.valid) return path;

    return PathInfo();  // 无路径
}

// ============================================================================
// 移除一对方块
// ============================================================================
void GameBoard::removeTiles(int r1, int c1, int r2, int c2)
{
    m_grid[r1][c1] = EMPTY;
    m_grid[r2][c2] = EMPTY;
    m_remainingTiles -= 2;
    m_moves++;
}

// ============================================================================
// 查找提示：遍历所有配对，找第一个可连接的
// ============================================================================
PathInfo GameBoard::findHint() const
{
    // 先按类型收集所有方块位置
    QVector<QPoint> positionsByType[TILE_TYPES + 1];
    for (int r = 1; r <= ROWS; ++r)
    {
        for (int c = 1; c <= COLS; ++c)
        {
            int type = m_grid[r][c];
            if (type != EMPTY)
                positionsByType[type].append(QPoint(c, r));
        }
    }

    // 对每种类型，检查所有配对
    for (int type = 1; type <= TILE_TYPES; ++type)
    {
        const QVector<QPoint>& positions = positionsByType[type];
        int count = positions.size();
        for (int i = 0; i < count; ++i)
        {
            for (int j = i + 1; j < count; ++j)
            {
                int r1 = positions[i].y(), c1 = positions[i].x();
                int r2 = positions[j].y(), c2 = positions[j].x();
                PathInfo path = findPath(r1, c1, r2, c2);
                if (path.valid)
                    return path;
            }
        }
    }

    return PathInfo();  // 无可消除配对
}

// ============================================================================
// 是否还有可消除的配对
// ============================================================================
bool GameBoard::hasValidMoves() const
{
    return findHint().valid;
}

// ============================================================================
// 是否通关
// ============================================================================
bool GameBoard::isWin() const
{
    return m_remainingTiles == 0;
}

// ============================================================================
// 重排剩余方块
// ============================================================================
void GameBoard::shuffle()
{
    // 收集所有非空方块
    QVector<int> remaining;
    remaining.reserve(m_remainingTiles);
    for (int r = 1; r <= ROWS; ++r)
    {
        for (int c = 1; c <= COLS; ++c)
        {
            if (m_grid[r][c] != EMPTY)
                remaining.append(m_grid[r][c]);
        }
    }

    // 随机打乱
    std::shuffle(remaining.begin(), remaining.end(), m_rng);

    // 放回棋盘
    int idx = 0;
    for (int r = 1; r <= ROWS; ++r)
    {
        for (int c = 1; c <= COLS; ++c)
        {
            if (m_grid[r][c] != EMPTY)
            {
                m_grid[r][c] = remaining[idx++];
            }
        }
    }
}
