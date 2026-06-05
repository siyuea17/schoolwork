// ============================================================================
// 文件：GameBoard.cpp
// 角色：GameBoard 类的实现文件——包含连连看所有核心算法
// ============================================================================

#include "GameBoard.h"
#include <QVector>
#include <QDateTime>
#include <algorithm>
#include <random>
#include <chrono>

// ============================================================================
// 构造函数 —— 动态维度版本
//
// 参数：
//   rows —— 实际游戏行数（默认8）
//   cols —— 实际游戏列数（默认10）
//   tileTypes —— 图案种类数（默认20）
//   copiesPerType —— 每种图案的副本数（默认4→20×4=80方块）
//
// 注意：棋盘总大小还要在四周各加一行/列边界（用于路径绕过）
// ============================================================================
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
    // ===== 初始化棋盘网格 =====
    // 分配 totalRows × totalCols 的二维向量
    int tr = totalRows();
    int tc = totalCols();
    m_grid.resize(tr);
    for (int r = 0; r < tr; ++r)
    {
        m_grid[r].resize(tc);
        for (int c = 0; c < tc; ++c)
            m_grid[r][c] = EMPTY;
    }

    // ===== 初始化随机数生成器 =====
    // 用当前时间的毫秒数做种子，每次运行棋盘布局不同
    m_rng.seed(static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
}

// ============================================================================
// initBoard() —— 生成一个全新的随机棋盘
//
// 算法：
//   1. 生成方块列表：每种图案 copiesPerType 个副本
//   2. 随机打乱
//   3. 按行逐列放置到游戏区域
//   4. 确保边界全是空格
// ============================================================================
void GameBoard::initBoard()
{
    int tr = totalRows();
    int tc = totalCols();

    // ---- 第1步：生成方块列表 ----
    // 总方块数 = rows × cols = 图案种类 × 每类副本
    int total = m_rows * m_cols;
    QVector<int> tiles;
    tiles.reserve(total);

    for (int type = 1; type <= m_tileTypes; ++type)
    {
        for (int i = 0; i < m_copiesPerType; ++i)
            tiles.append(type);
    }
    // 总方块数对不上的情况（参数不当）→ 按实际填充
    while (tiles.size() < total)
        tiles.append(1);

    // ---- 第2步：随机打乱 ----
    std::shuffle(tiles.begin(), tiles.end(), m_rng);

    // ---- 第3步：放置到棋盘 ----
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

    // ---- 第4步：清空边界 ----
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

    // ---- 第5步：重置游戏状态 ----
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

// ============================================================================
// isRowClear() —— 判断同一行上两点之间是否畅通无阻
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

// ============================================================================
// isColClear() —— 判断同一列上两点之间是否畅通无阻
// ============================================================================
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
// tryDirectLink() —— "0折连接"（直线连接）
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
// tryOneTurnLink() —— "1折连接"（L形连接，拐一个弯）
// ============================================================================
PathInfo GameBoard::tryOneTurnLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;

    // 拐角方案1：拐点在 (r1, c2)
    if (m_grid[r1][c2] == EMPTY)
    {
        if (isRowClear(r1, c1, c2) && isColClear(r1, r2, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1), QPoint(c2, r1), QPoint(c2, r2) };
            return result;
        }
    }

    // 拐角方案2：拐点在 (r2, c1)
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
// tryTwoTurnLink() —— "2折连接"（Z形或U形，拐两个弯）
//
// 核心思路：扫描所有可能的"中间行"和"中间列"
//   扫描中间行：竖→横→竖
//   扫描中间列：横→竖→横
// ============================================================================
PathInfo GameBoard::tryTwoTurnLink(int r1, int c1, int r2, int c2) const
{
    int tr = totalRows();
    int tc = totalCols();
    PathInfo result;

    // ===== 扫描所有可能的中间行 =====
    for (int r = 0; r < tr; ++r)
    {
        if (r == r1 || r == r2) continue;

        if (m_grid[r][c1] != EMPTY) continue;
        if (m_grid[r][c2] != EMPTY) continue;

        if (isColClear(r1, r, c1) && isRowClear(r, c1, c2) && isColClear(r, r2, c2))
        {
            result.valid = true;
            result.corners = { QPoint(c1, r1), QPoint(c1, r), QPoint(c2, r), QPoint(c2, r2) };
            return result;
        }
    }

    // ===== 扫描所有可能的中间列 =====
    for (int c = 0; c < tc; ++c)
    {
        if (c == c1 || c == c2) continue;

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
// findPath() —— 主路径查找函数（对外接口）
//
// 按顺序尝试 0折 → 1折 → 2折（越简单越优先）
// ============================================================================
PathInfo GameBoard::findPath(int r1, int c1, int r2, int c2) const
{
    int tr = totalRows();
    int tc = totalCols();

    if (r1 < 0 || r1 >= tr || c1 < 0 || c1 >= tc) return PathInfo();
    if (r2 < 0 || r2 >= tr || c2 < 0 || c2 >= tc) return PathInfo();

    if (m_grid[r1][c1] == EMPTY || m_grid[r2][c2] == EMPTY) return PathInfo();
    if (m_grid[r1][c1] != m_grid[r2][c2]) return PathInfo();
    if (r1 == r2 && c1 == c2) return PathInfo();

    PathInfo path = tryDirectLink(r1, c1, r2, c2);
    if (path.valid) return path;

    path = tryOneTurnLink(r1, c1, r2, c2);
    if (path.valid) return path;

    path = tryTwoTurnLink(r1, c1, r2, c2);
    if (path.valid) return path;

    return PathInfo();
}

// ============================================================================
// removeTiles() —— 消除一对匹配的方块
//
// 副作用：
//   - m_remainingTiles 减少2
//   - m_moves 增加1
//   - 记录消除时间戳用于连击判断
// ============================================================================
void GameBoard::removeTiles(int r1, int c1, int r2, int c2)
{
    m_grid[r1][c1] = EMPTY;
    m_grid[r2][c2] = EMPTY;
    m_remainingTiles -= 2;
    m_moves++;
}

// ============================================================================
// calculateComboScore() —— 计算连击分
//
// 连击机制：
//   - 如果上一次消除距今 ≤ 3 秒 → 连击+1
//   - 如果超过 3 秒 → 连击归零重新开始
//   - 基础分 10，每连击一次 +5
//   - 公式：comboScore = 10 + comboCount × 5
//
// 举例：玩家3秒内连续消除3对
//   第1对：连击0 → 10分
//   第2对：连击1 → 15分（10 + 1×5）
//   第3对：连击2 → 20分（10 + 2×5）
//   总计：45分（比没有连击系统多15分加成）
//
// 注意：这个函数在 removeTiles() 之前调用，
//       因为需要比较"上一次"的时间戳来判断连击。
// ============================================================================
int GameBoard::calculateComboScore()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // 如果这不是第一次消除，检查是否在连击窗口内
    if (m_moves > 0 && m_lastMatchTimeMs > 0)
    {
        qint64 elapsed = now - m_lastMatchTimeMs;
        if (elapsed <= COMBO_WINDOW_MS)
        {
            // 在窗口内 → 连击继续
            m_comboCount++;
        }
        else
        {
            // 超时 → 连击中断重新开始
            m_comboCount = 0;
        }
    }

    // 记录本次消除时间
    m_lastMatchTimeMs = now;

    // 基础分 10，连击加成每级 +5
    int baseScore = 10;
    int bonus = m_comboCount * 5;
    return baseScore + bonus;
}

// ============================================================================
// findHint() —— 找一对可以消除的方块（用于"提示"功能）
//
// 优化：先按类型分组，再组内两两配对查找。
//   比如标准难度20种图案每组4个，只需 20×6=120 个配对检查，
//   远少于全部 80×79÷2=3160 个。
// ============================================================================
PathInfo GameBoard::findHint() const
{
    // 收集所有方块位置，按类型分组
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

    // 对每种类型，两两尝试配对
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

// ============================================================================
// hasValidMoves() —— 判断棋盘上是否还存在可消除的配对
// ============================================================================
bool GameBoard::hasValidMoves() const
{
    return findHint().valid;
}

// ============================================================================
// isWin() —— 判断是否通关
// ============================================================================
bool GameBoard::isWin() const
{
    return m_remainingTiles == 0;
}

// ============================================================================
// shuffle() —— 重排剩余方块
//
// 只改变方块的图案分布，不改变空位的位置。
// 空位还是空位，方块图案被随机洗牌。
// ============================================================================
void GameBoard::shuffle()
{
    // 收集所有剩余方块
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

    // 随机打乱
    std::shuffle(remaining.begin(), remaining.end(), m_rng);

    // 按新顺序放回
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

// ============================================================================
// serializeGrid() —— 棋盘数据序列化
//
// 把 totalRows × totalCols 的二维网格展平成一维整数数组。
// 包含边界（因为存档要完整恢复棋盘状态）。
// 返回值可直接存入 QSettings。
// ============================================================================
QVector<int> GameBoard::serializeGrid() const
{
    int tr = totalRows();
    int tc = totalCols();
    QVector<int> data;
    data.reserve(tr * tc + 4);

    // 前4个元素存储棋盘尺寸信息，恢复时用
    data.append(m_rows);
    data.append(m_cols);
    data.append(m_tileTypes);
    data.append(m_copiesPerType);

    for (int r = 0; r < tr; ++r)
        for (int c = 0; c < tc; ++c)
            data.append(m_grid[r][c]);

    return data;
}

// ============================================================================
// deserializeGrid() —— 从序列化数据恢复棋盘
//
// 从 serializeGrid() 导出的数据重建整个网格。
// 注意：不恢复分数/步数/连击，那些由 GameWidget/MainWindow 管理。
// ============================================================================
void GameBoard::deserializeGrid(const QVector<int>& data)
{
    if (data.size() < 4) return;

    // 读取棋盘参数
    m_rows = data[0];
    m_cols = data[1];
    m_tileTypes = data[2];
    m_copiesPerType = data[3];

    int tr = totalRows();
    int tc = totalCols();

    // 重新分配网格
    m_grid.clear();
    m_grid.resize(tr);
    for (int r = 0; r < tr; ++r)
        m_grid[r].resize(tc);

    // 填充数据
    int idx = 4;
    for (int r = 0; r < tr; ++r)
        for (int c = 0; c < tc; ++c)
            m_grid[r][c] = (idx < data.size()) ? data[idx++] : EMPTY;

    // 重新计算剩余方块数
    m_remainingTiles = 0;
    for (int r = 1; r <= m_rows; ++r)
        for (int c = 1; c <= m_cols; ++c)
            if (m_grid[r][c] != EMPTY)
                m_remainingTiles++;
}
