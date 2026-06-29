// GameBoard.cpp — 连连看棋盘逻辑与路径查找

#include "GameBoard.h"

#include <QVector>
#include <QDateTime>
#include <algorithm>
#include <random>
#include <chrono>

// ---- 构造 ----

GameBoard::GameBoard(int rows, int cols, int tileTypes, int copiesPerType)
    : m_rows(rows)
    , m_cols(cols)
    , m_tileTypes(tileTypes)
    , m_copiesPerType(copiesPerType)
    , m_score(0)
    , m_remainingTiles(0)
    , m_moves(0)
    , m_comboCount(0)
    , m_lastMatchTimeMs(0)
{
    int tr = totalRows();
    int tc = totalCols();

    m_grid.resize(tr);
    for (int r = 0; r < tr; ++r)
    {
        m_grid[r].resize(tc);
        for (int c = 0; c < tc; ++c)
            m_grid[r][c] = EMPTY;
    }

    m_rng.seed(static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
}

// ---- 棋盘初始化与重置 ----

void GameBoard::initBoard()
{
    int tr = totalRows();
    int tc = totalCols();

    // 生成方块列表：每种图案 copiesPerType 个副本
    int total = m_rows * m_cols;
    QVector<int> tiles;
    tiles.reserve(total);

    for (int type = 1; type <= m_tileTypes; ++type)
    {
        for (int i = 0; i < m_copiesPerType; ++i)
            tiles.append(type);
    }

    while (tiles.size() < total)
        tiles.append(1);

    // 随机打乱并放到棋盘上
    std::shuffle(tiles.begin(), tiles.end(), m_rng);

    int index = 0;
    for (int r = 1; r <= m_rows; ++r)
    {
        for (int c = 1; c <= m_cols; ++c)
        {
            if (index < tiles.size())
                m_grid[r][c] = tiles[index++];
            else
                m_grid[r][c] = EMPTY;
        }
    }

    // 确保边界行和边界列为空格（路径可以通过边界外面绕过去）
    for (int r = 0; r < tr; ++r)
    {
        m_grid[r][0] = EMPTY;
        m_grid[r][tc - 1] = EMPTY;
    }
    for (int c = 0; c < tc; ++c)
    {
        m_grid[0][c] = EMPTY;
        m_grid[tr - 1][c] = EMPTY;
    }

    m_score = 0;
    m_remainingTiles = total;
    m_moves = 0;
    m_comboCount = 0;
    m_lastMatchTimeMs = 0;
}

void GameBoard::reset()
{
    initBoard();
}

// ---- 路径通畅性检查（路径查找的基础） ----

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

// ---- 路径查找：0折 / 1折 / 2折，由简到繁依次尝试 ----

// 0折：同行或同列，中间无阻挡即可直线连接
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

// 1折：L形路径，在 (r1,c2) 或 (r2,c1) 处拐弯，拐角必须为空
PathInfo GameBoard::tryOneTurnLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;

    // 拐角方案1：拐角在 (r1, c2) —— 先横后竖
    if (m_grid[r1][c2] == EMPTY)
    {
        if (isRowClear(r1, c1, c2) &&
            isColClear(r1, r2, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1),
                               QPoint(c2, r1),
                               QPoint(c2, r2) };
            return result;
        }
    }

    // 拐角方案2：拐角在 (r2, c1) —— 先竖后横
    if (m_grid[r2][c1] == EMPTY)
    {
        if (isColClear(r1, r2, c1) &&
            isRowClear(r2, c1, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1),
                               QPoint(c1, r2),
                               QPoint(c2, r2) };
            return result;
        }
    }

    return result;
}

// 2折：Z/U形路径，扫描所有可能的中间行和中间列（含边界，允许从棋盘外绕行）
PathInfo GameBoard::tryTwoTurnLink(int r1, int c1, int r2, int c2) const
{
    int tr = totalRows();
    int tc = totalCols();
    PathInfo result;

    // 扫描所有可能的"中间行"
    for (int r = 0; r < tr; ++r)
    {
        if (r == r1 || r == r2) continue;
        if (m_grid[r][c1] != EMPTY) continue;
        if (m_grid[r][c2] != EMPTY) continue;

        if (isColClear(r1, r, c1) &&
            isRowClear(r, c1, c2) &&
            isColClear(r, r2, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1),
                               QPoint(c1, r),
                               QPoint(c2, r),
                               QPoint(c2, r2) };
            return result;
        }
    }

    // 扫描所有可能的"中间列"
    for (int c = 0; c < tc; ++c)
    {
        if (c == c1 || c == c2) continue;
        if (m_grid[r1][c] != EMPTY) continue;
        if (m_grid[r2][c] != EMPTY) continue;

        if (isRowClear(r1, c1, c) &&
            isColClear(r1, r2, c) &&
            isRowClear(r2, c, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1),
                               QPoint(c, r1),
                               QPoint(c, r2),
                               QPoint(c2, r2) };
            return result;
        }
    }

    return result;
}

// 按 0折→1折→2折 的顺序依次尝试，找到任意一种即返回
PathInfo GameBoard::findPath(int r1, int c1, int r2, int c2) const
{
    int tr = totalRows();
    int tc = totalCols();

    if (r1 < 0 || r1 >= tr || c1 < 0 || c1 >= tc)
        return PathInfo();
    if (r2 < 0 || r2 >= tr || c2 < 0 || c2 >= tc)
        return PathInfo();
    if (m_grid[r1][c1] == EMPTY || m_grid[r2][c2] == EMPTY)
        return PathInfo();
    if (m_grid[r1][c1] != m_grid[r2][c2])
        return PathInfo();
    if (r1 == r2 && c1 == c2)
        return PathInfo();

    PathInfo path = tryDirectLink(r1, c1, r2, c2);
    if (path.valid) return path;

    path = tryOneTurnLink(r1, c1, r2, c2);
    if (path.valid) return path;

    path = tryTwoTurnLink(r1, c1, r2, c2);
    if (path.valid) return path;

    return PathInfo();
}

// ---- 消除与计分 ----

void GameBoard::removeTiles(int r1, int c1, int r2, int c2)
{
    m_grid[r1][c1] = EMPTY;
    m_grid[r2][c2] = EMPTY;
    m_remainingTiles -= 2;
    m_moves++;
}

// 连击机制：5秒内连续消除有加成（每级+5分），超时归零
int GameBoard::calculateComboScore()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    if (m_moves > 0 && m_lastMatchTimeMs > 0)
    {
        qint64 elapsed = now - m_lastMatchTimeMs;
        if (elapsed <= COMBO_WINDOW_MS)
        {
            m_comboCount++;
        }
        else
        {
            m_comboCount = 0;
        }
    }

    m_lastMatchTimeMs = now;

    int baseScore = 10;
    int bonus = m_comboCount * 5;
    return baseScore + bonus;
}

// ---- 提示与状态查询 ----

// 按图案类型分组后组内配对查找，避免 O(n^2) 的全局两两比对
PathInfo GameBoard::findHint() const
{
    QVector<QVector<QPoint>> positionsByType(m_tileTypes + 1);

    for (int r = 1; r <= m_rows; ++r)
    {
        for (int c = 1; c <= m_cols; ++c)
        {
            int type = m_grid[r][c];
            if (type != EMPTY)
                positionsByType[type].append(QPoint(c, r));
        }
    }

    for (int type = 1; type <= m_tileTypes; ++type)
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

    return PathInfo();
}

bool GameBoard::hasValidMoves() const
{
    return findHint().valid;
}

bool GameBoard::isWin() const
{
    return m_remainingTiles == 0;
}

// ---- 重排 ----

// 只改变方块图案的分布，不改变"哪些位置有方块"
void GameBoard::shuffle()
{
    QVector<int> remaining;
    remaining.reserve(m_remainingTiles);
    for (int r = 1; r <= m_rows; ++r)
    {
        for (int c = 1; c <= m_cols; ++c)
        {
            if (m_grid[r][c] != EMPTY)
                remaining.append(m_grid[r][c]);
        }
    }

    std::shuffle(remaining.begin(), remaining.end(), m_rng);

    int idx = 0;
    for (int r = 1; r <= m_rows; ++r)
    {
        for (int c = 1; c <= m_cols; ++c)
        {
            if (m_grid[r][c] != EMPTY)
                m_grid[r][c] = remaining[idx++];
        }
    }
}

// ---- 序列化 / 反序列化 ----

QVector<int> GameBoard::serializeGrid() const
{
    int tr = totalRows();
    int tc = totalCols();
    QVector<int> data;
    data.reserve(tr * tc + 4);

    data.append(m_rows);
    data.append(m_cols);
    data.append(m_tileTypes);
    data.append(m_copiesPerType);

    for (int r = 0; r < tr; ++r)
        for (int c = 0; c < tc; ++c)
            data.append(m_grid[r][c]);

    return data;
}

void GameBoard::deserializeGrid(const QVector<int>& data)
{
    if (data.size() < 4) return;

    m_rows = data[0];
    m_cols = data[1];
    m_tileTypes = data[2];
    m_copiesPerType = data[3];

    int tr = totalRows();
    int tc = totalCols();

    m_grid.clear();
    m_grid.resize(tr);
    for (int r = 0; r < tr; ++r)
        m_grid[r].resize(tc);

    int idx = 4;
    for (int r = 0; r < tr; ++r)
        for (int c = 0; c < tc; ++c)
            m_grid[r][c] = (idx < data.size()) ? data[idx++] : EMPTY;

    m_remainingTiles = 0;
    for (int r = 1; r <= m_rows; ++r)
        for (int c = 1; c <= m_cols; ++c)
            if (m_grid[r][c] != EMPTY)
                m_remainingTiles++;
}
