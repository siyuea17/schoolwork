// ============================================================================
// 文件：GameBoard.cpp
// 角色：连连看的"大脑"——棋盘逻辑、路径查找算法、连击计分
//
// 本文件是纯 C++ 逻辑，不涉及任何界面绘制。
// 每一行代码都配有"如果……就……"风格的自然语言注释，方便初学理解。
// ============================================================================

#include "GameBoard.h"

// ---- Qt 头文件（Qt 是开发桌面应用的框架，提供界面、定时器等功能） ----
#include <QVector>        // QVector —— Qt 的动态数组，类似 C++ 的 std::vector，但和 Qt 生态无缝集成
#include <QDateTime>      // QDateTime —— Qt 的日期时间类，可以获取当前时间的毫秒数
#include <algorithm>      // algorithm —— C++ 标准库的算法头文件，提供 std::shuffle（洗牌）等工具
#include <random>         // random —— C++ 标准库的随机数头文件，提供高质量的随机数引擎
#include <chrono>         // chrono —— C++ 标准库的时间头文件，用于获取当前时刻作为随机种子

// ============================================================================
// 构造函数 —— 创建棋盘对象时自动调用
// 参数：rows=游戏行数, cols=游戏列数, tileTypes=图案种类数, copiesPerType=每种图案有几个
// ============================================================================
GameBoard::GameBoard(int rows, int cols, int tileTypes, int copiesPerType)
    : m_rows(rows)              // 把传入的游戏行数存到成员变量
    , m_cols(cols)              // 把传入的游戏列数存到成员变量
    , m_tileTypes(tileTypes)    // 把传入的图案种类数存到成员变量
    , m_copiesPerType(copiesPerType) // 把传入的每种图案的副本数存到成员变量
    , m_score(0)                // 初始分数为 0
    , m_remainingTiles(0)       // 剩余方块数暂设为 0（initBoard 里会改）
    , m_moves(0)                // 初始步数为 0
    , m_comboCount(0)           // 初始连击数为 0
    , m_lastMatchTimeMs(0)      // 上一次消除的时间戳暂设为 0
{
    // ---- 计算含边界的总行数（实际行数 + 上下各1行边界） ----
    int tr = totalRows();       // totalRows() 返回 m_rows + 2
    int tc = totalCols();       // totalCols() 返回 m_cols + 2

    // ---- 分配二维数组的内存（QVector 管理，自动释放） ----
    m_grid.resize(tr);          // 先分配 tr 行
    for (int r = 0; r < tr; ++r) // 对于每一行
    {
        m_grid[r].resize(tc);   // 这一行分配 tc 列
        for (int c = 0; c < tc; ++c) // 对于这一行的每一列
            m_grid[r][c] = EMPTY;    // 把这个格子设为 0（空格），棋盘初始为空
    }

    // ---- 初始化随机数引擎（每次运行产生不同的随机序列） ----
    // std::chrono::steady_clock::now() 获取"当前时刻"的某个高精度计时点
    // .time_since_epoch() 计算从计时起点到现在的时长
    // .count() 把时长转为一个数字（纳秒）
    // static_cast<unsigned int>() 把这个数字转为无符号整数——作为随机数种子
    // 因为时间是永远向前的，所以每次运行种子都不同，从而每次生成的棋盘都不同
    m_rng.seed(static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
}

// ============================================================================
// initBoard() —— 生成一个全新的随机棋盘布局
//
// 算法思路：
//   首先，创建一个列表，每种图案放 copiesPerType 个（如中等难度：20种图案各4个=80个）
//   然后，用洗牌算法把这个列表的顺序打乱
//   最后，按行按列把打乱后的方块放到游戏区域（跳过边界）
// ============================================================================
void GameBoard::initBoard()
{
    int tr = totalRows();       // 获取含边界的总行数
    int tc = totalCols();       // 获取含边界的总列数

    // ===== 第1步：生成方块列表 =====
    // 用 QVector<int> 存储方块编号（1到tileTypes，0代表空格）
    int total = m_rows * m_cols;         // 计算游戏区域总格子数（如 8×10=80）
    QVector<int> tiles;                  // 创建一个空的方块编号列表
    tiles.reserve(total);                // 提前预留足够的内存空间，避免插入时反复扩容

    // 对于每一种图案（从第1种到第 tileTypes 种）
    for (int type = 1; type <= m_tileTypes; ++type)
    {
        // 为这种图案生成 copiesPerType 个副本（如默认4个）
        for (int i = 0; i < m_copiesPerType; ++i)
            tiles.append(type);          // 把图案编号加入列表
    }

    // 如果方块总数不够填满棋盘（极端情况，一般不会发生），用第1种图案补足
    while (tiles.size() < total)         // 当列表中的方块数 < 需要的总数时
        tiles.append(1);                 // 继续往列表末尾添加第1种图案

    // ===== 第2步：随机打乱方块顺序 =====
    // std::shuffle 是 C++ 标准库的洗牌函数，效果等同于洗扑克牌
    //   tiles.begin() → 要从哪里开始洗（列表开头）
    //   tiles.end()   → 要洗到哪里结束（列表末尾）
    //   m_rng         → 用哪个随机数引擎（构造函数里初始化过了）
    std::shuffle(tiles.begin(), tiles.end(), m_rng);

    // ===== 第3步：把打乱后的方块逐个放到棋盘上 =====
    int index = 0;                       // 指向 tiles 中"下一个要放的方块"的索引
    for (int r = 1; r <= m_rows; ++r)   // 遍历游戏区域的每一行（从第1行开始，跳过第0行边界）
    {
        for (int c = 1; c <= m_cols; ++c) // 遍历这一行的每一列（跳过第0列边界）
        {
            if (index < tiles.size())    // 如果列表中还有方块没放
                m_grid[r][c] = tiles[index++]; // 取出一个方块放到这个位置，然后索引+1
            else                         // 否则（列表已空）
                m_grid[r][c] = EMPTY;    // 这个位置设为空格
        }
    }

    // ===== 第4步：确保边界行和边界列全部是空格 =====
    // 左边界（第0列）和右边界（最后一列）
    for (int r = 0; r < tr; ++r)         // 遍历所有行
    {
        m_grid[r][0] = EMPTY;            // 左边界设为空格（因为路径可以通过边界外面绕过去）
        m_grid[r][tc - 1] = EMPTY;       // 右边界也设为空格
    }
    // 上边界（第0行）和下边界（最后一行）
    for (int c = 0; c < tc; ++c)         // 遍历所有列
    {
        m_grid[0][c] = EMPTY;            // 上边界设为空格
        m_grid[tr - 1][c] = EMPTY;       // 下边界设为空格
    }

    // ===== 第5步：重置游戏状态为初始值 =====
    m_score = 0;                         // 分数归零
    m_remainingTiles = total;            // 剩余方块数 = 总数（全部方块都在）
    m_moves = 0;                         // 步数归零
    m_comboCount = 0;                    // 连击数归零
    m_lastMatchTimeMs = 0;               // 上次消除时间戳归零
}

// ============================================================================
// reset() —— 重置游戏（等于重新生成棋盘）
// 这是一个"快捷方式"函数，只是换了个名字方便外部调用
// ============================================================================
void GameBoard::reset()
{
    initBoard();  // 直接调用 initBoard 生成全新随机棋盘
}

// ============================================================================
// isRowClear() —— 判断同一行上，两个列号之间是否没有任何阻碍（全是空格）
// 这个函数是路径查找的基础砖块——所有连接方式都要用它来检查"能不能直走"
//
// 参数：r=行号, c1=第一个列号, c2=第二个列号
// 返回：true=之间全空畅通, false=中间有方块阻挡
// ============================================================================
bool GameBoard::isRowClear(int r, int c1, int c2) const
{
    // 用三元运算符找出较小的列号和较大的列号
    // 语法：(条件) ? 值1 : 值2   →   如果条件成立取值1，否则取值2
    int minC = (c1 < c2) ? c1 : c2;      // 如果 c1 < c2，则 minC = c1；否则 minC = c2
    int maxC = (c1 < c2) ? c2 : c1;      // 如果 c1 < c2，则 maxC = c2；否则 maxC = c1

    // 从较小的列号+1 开始检查，一直检查到较大的列号-1
    // 为什么从+1开始？因为端点本身是方块，不能算作"阻挡自己"
    for (int c = minC + 1; c < maxC; ++c) // 对于中间的每一个格子
    {
        if (m_grid[r][c] != EMPTY)        // 如果这个格子不是空格（被方块占据了）
            return false;                  // 就说明有阻挡，路径不通，返回 false
    }
    return true;                           // 所有中间格子都是空格，畅通无阻！
}

// ============================================================================
// isColClear() —— 判断同一列上，两个行号之间是否畅通无阻
// 功能和 isRowClear 一样，只是方向从水平变成了垂直
// ============================================================================
bool GameBoard::isColClear(int r1, int r2, int c) const
{
    int minR = (r1 < r2) ? r1 : r2;      // 找出较小的行号
    int maxR = (r1 < r2) ? r2 : r1;      // 找出较大的行号

    for (int r = minR + 1; r < maxR; ++r) // 对于中间每一行
    {
        if (m_grid[r][c] != EMPTY)        // 如果这个格子不是空格
            return false;                  // 就说明有阻挡，返回 false
    }
    return true;                           // 全部畅通，返回 true
}

// ============================================================================
// tryDirectLink() —— 尝试0折连接（直线连接，不用拐弯）
//
// 这是最简单的连接方式——如果两个方块在同一行或同一列，且中间所有格子都是空的，
// 就可以把这两个方块连起来消除。
//
// 比如两个方块在同一行，中间没有其他方块挡着 → 可以连。
// 如果一个在上一个在下，中间隔着其他方块 → 不能直线连。
// ============================================================================
PathInfo GameBoard::tryDirectLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;  // 创建路径信息结构体，默认 valid = false

    // 情况A：两个方块在同一行，且它们之间没有阻挡
    if (r1 == r2 && isRowClear(r1, c1, c2))   // 如果 r1 等于 r2（同在一行）并且这一行两点之间畅通
    {
        result.valid = true;                   // 就标记为"找到了有效路径"
        result.corners = { QPoint(c1, r1), QPoint(c2, r2) };
        // QPoint(x=列, y=行) 是 Qt 的坐标类——用来存储一个点在网格中的位置
        // 0折路径只有2个点：起点和终点，中间不需要拐弯
    }
    // 情况B：两个方块在同一列，且它们之间没有阻挡
    else if (c1 == c2 && isColClear(r1, r2, c1)) // 否则如果 c1 等于 c2（同在一列）并且这一列两点之间畅通
    {
        result.valid = true;                   // 标记为有效
        result.corners = { QPoint(c1, r1), QPoint(c2, r2) };
    }
    // 否则（既不同行也不同列，或者同行/同列但有阻挡）→ valid 保持 false

    return result;  // 把路径信息返回给调用者（可能是有效路径，也可能是无效的）
}

// ============================================================================
// tryOneTurnLink() —— 尝试1折连接（拐一个弯，像 L 形）
//
// 0折连接失败后，进一步尝试1折连接。
// 1折路径像一个大写字母 L：先横着走一段，然后拐弯竖着走到目标。
// 或者先竖着走一段，然后拐弯横着走到目标。
//
// 拐弯处必须是空格——因为"穿过方块"是不行的。
// ============================================================================
PathInfo GameBoard::tryOneTurnLink(int r1, int c1, int r2, int c2) const
{
    PathInfo result;  // 创建路径信息，默认 valid = false

    // ---- 拐角方案1：拐角在 (r1, c2) ----
    // 意思是：从起点 (r1,c1) 先横着走到 (r1,c2)，再竖着走到终点 (r2,c2)
    // 拐角 (r1,c2) 必须在棋盘上为空（因为路径要通过这个位置）
    if (m_grid[r1][c2] == EMPTY)               // 如果拐角位置是空格
    {
        if (isRowClear(r1, c1, c2) &&          // 并且水平段（r1行从c1到c2）畅通
            isColClear(r1, r2, c2))            // 并且垂直段（c2列从r1到r2）畅通
        {
            result.valid = true;                // 就找到了有效路径！
            result.corners = { QPoint(c1, r1),  //   起点
                               QPoint(c2, r1),  //   拐角（在起点同行、终点同列处拐弯）
                               QPoint(c2, r2) };//   终点
            return result;                      // 找到路径，直接返回，不用再尝试方案2了
        }
    }

    // ---- 拐角方案2：拐角在 (r2, c1) ----
    // 意思是：从起点 (r1,c1) 先竖着走到 (r2,c1)，再横着走到终点 (r2,c2)
    if (m_grid[r2][c1] == EMPTY)               // 如果拐角位置是空格
    {
        if (isColClear(r1, r2, c1) &&          // 并且垂直段（c1列从r1到r2）畅通
            isRowClear(r2, c1, c2))            // 并且水平段（r2行从c1到c2）畅通
        {
            result.valid = true;                // 找到了！
            result.corners = { QPoint(c1, r1),  //   起点
                               QPoint(c1, r2),  //   拐角
                               QPoint(c2, r2) };//   终点
            return result;                      // 返回
        }
    }

    return result;  // 两个拐角方案都不通，返回 valid=false
}

// ============================================================================
// tryTwoTurnLink() —— 尝试2折连接（拐两个弯，像 Z 形或 U 形）
//
// 0折和1折都失败时，这是最后的机会。2折路径是：先往一个方向走，再横着走，再往目标方向走。
// 形成的路径像"Z"字或"U"字形状。
//
// 核心思想：对于棋盘上的每一行（包括边界行），检查是否能以这一行作为"中间通道"；
// 对于棋盘上的每一列（包括边界列），检查是否能以这一列作为"中间通道"。
// 边界行和列也参与检查——因为连连看允许路径从棋盘"外面"绕过去。
// ============================================================================
PathInfo GameBoard::tryTwoTurnLink(int r1, int c1, int r2, int c2) const
{
    int tr = totalRows();  // 含边界的总行数
    int tc = totalCols();  // 含边界的总列数
    PathInfo result;

    // ===== 扫描所有可能的"中间行"——作为水平的Z形通道 =====
    // 对于每一行 r（从第0行到最后一行的所有行）
    for (int r = 0; r < tr; ++r)
    {
        if (r == r1 || r == r2) continue;      // 如果这一行就是起点行或终点行，就跳过（这些情况已经被0折和1折处理过了）

        if (m_grid[r][c1] != EMPTY) continue;  // 如果中间行上的左端点有方块，就跳过——路径不能穿过方块
        if (m_grid[r][c2] != EMPTY) continue;  // 如果中间行上的右端点有方块，也跳过

        // 检查三段路径是否都畅通：
        // 竖段1：从起点往下走到中间行（在同一列上）
        // 横段：在中间行上从左走到右
        // 竖段2：从中间行再往下走到终点（在同一列上）
        if (isColClear(r1, r, c1) &&            // 如果竖段1（起点到中间行的c1列）畅通
            isRowClear(r, c1, c2) &&             // 并且横段（中间行上从c1到c2）畅通
            isColClear(r, r2, c2))               // 并且竖段2（中间行到终点的c2列）畅通
        {
            result.valid = true;                 // 就找到了有效路径！
            result.corners = { QPoint(c1, r1),   //   起点
                               QPoint(c1, r),    //   拐角1（竖段1的终点，从这开始横向走）
                               QPoint(c2, r),    //   拐角2（横段的终点，从这开始往下走）
                               QPoint(c2, r2) }; //   终点
            return result;                       // 找到一条路径就立即返回
        }
    }

    // ===== 扫描所有可能的"中间列"——作为垂直的Z形通道 =====
    // 对于每一列 c
    for (int c = 0; c < tc; ++c)
    {
        if (c == c1 || c == c2) continue;      // 如果这一列就是起点列或终点列，跳过

        if (m_grid[r1][c] != EMPTY) continue;  // 如果中间列上的上端点有方块，跳过
        if (m_grid[r2][c] != EMPTY) continue;  // 如果中间列上的下端点有方块，跳过

        if (isRowClear(r1, c1, c) &&            // 如果横段1畅通
            isColClear(r1, r2, c) &&             // 并且竖段畅通
            isRowClear(r2, c, c2))               // 并且横段2畅通
        {
            result.valid = true;                 // 找到了！
            result.corners = { QPoint(c1, r1),   //   起点
                               QPoint(c, r1),    //   拐角1
                               QPoint(c, r2),    //   拐角2
                               QPoint(c2, r2) }; //   终点
            return result;
        }
    }

    return result;  // 扫描了所有中间行和中间列，都不通——这两个方块确实不能消除
}

// ============================================================================
// findPath() —— 主路径查找函数，是外部调用的"总入口"
//
// 它会按从简单到复杂的顺序依次尝试三种连接方式：
//   0折（直线）→ 1折（L形）→ 2折（Z形）
//   找到任何一种就立即返回，不用继续试更复杂的。
//
// 三种都失败 → 返回 valid=false，说明这两个方块不能连接。
// ============================================================================
PathInfo GameBoard::findPath(int r1, int c1, int r2, int c2) const
{
    int tr = totalRows();
    int tc = totalCols();

    // ---- 安全检查：坐标不能超出棋盘范围 ----
    if (r1 < 0 || r1 >= tr || c1 < 0 || c1 >= tc) // 如果第一个点的坐标超出棋盘范围
        return PathInfo();                          // 就返回无效路径（什么都不做）
    if (r2 < 0 || r2 >= tr || c2 < 0 || c2 >= tc) // 如果第二个点的坐标超出棋盘范围
        return PathInfo();                          // 就返回无效路径

    // ---- 安全检查：两个位置必须都有方块（不能是空格） ----
    if (m_grid[r1][c1] == EMPTY || m_grid[r2][c2] == EMPTY) // 如果任一位置为空
        return PathInfo();                         // 就返回无效路径

    // ---- 安全检查：两个方块必须是同一类型（图案相同才能消除） ----
    if (m_grid[r1][c1] != m_grid[r2][c2])          // 如果两个方块图案不同
        return PathInfo();                         // 就返回无效路径

    // ---- 安全检查：不能是同一个位置（自己和自己不能匹配） ----
    if (r1 == r2 && c1 == c2)                      // 如果两个坐标完全一致
        return PathInfo();                         // 就返回无效路径

    // ===== 按顺序尝试三种连接方式 =====
    // 利用了 C++ 的"短路逻辑"：如果前面返回了有效路径，后面就不会执行

    PathInfo path = tryDirectLink(r1, c1, r2, c2); // 首先尝试0折直线连接
    if (path.valid) return path;                   // 如果找到了，直接返回

    path = tryOneTurnLink(r1, c1, r2, c2);         // 0折失败的话，再尝试1折L形连接
    if (path.valid) return path;                   // 如果找到了，返回

    path = tryTwoTurnLink(r1, c1, r2, c2);         // 1折也失败，最后尝试2折Z形连接
    if (path.valid) return path;                   // 如果找到了，返回

    return PathInfo();  // 三种方式都失败了——这两个方块没法消除
}

// ============================================================================
// removeTiles() —— 真正消除两个方块（把两个位置变成空格）
// 注意：分数不在这里算——分数在 calculateComboScore() 里单独算
// ============================================================================
void GameBoard::removeTiles(int r1, int c1, int r2, int c2)
{
    m_grid[r1][c1] = EMPTY;            // 把第一个方块的位置设为空格（方块被消除）
    m_grid[r2][c2] = EMPTY;            // 把第二个方块的位置也设为空格
    m_remainingTiles -= 2;             // 剩余方块数减去2（一次性消除两个）
    m_moves++;                         // 步数加1（这次消除算一步）
}

// ============================================================================
// calculateComboScore() —— 计算本次消除应得的分数（含连击加成）
//
// 连击机制：如果玩家在5秒内连续消除方块，每多消一次就额外加5分。
// 比如3秒内消了3对：第1对10分 → 第2对15分 → 第3对20分 → 共45分
// 如果超过5秒没操作，连击归零重新开始。
// ============================================================================
int GameBoard::calculateComboScore()
{
    // ---- 获取当前时间（从1970年1月1日到现在的毫秒数） ----
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // ---- 如果不是第一次消除，检查距离上次消除过了多久 ----
    if (m_moves > 0 && m_lastMatchTimeMs > 0)  // 如果之前有过消除（步数>0）并且有时间记录
    {
        qint64 elapsed = now - m_lastMatchTimeMs; // 计算这次消除距上次过了多少毫秒
        if (elapsed <= COMBO_WINDOW_MS)           // 如果间隔在5秒以内
        {
            m_comboCount++;                        // 就连击加1
        }
        else                                       // 否则（间隔超过了5秒）
        {
            m_comboCount = 0;                      // 就连击归零，重新开始
        }
    }

    // ---- 不管有没有连击，都要更新"上次消除时间"为现在 ----
    m_lastMatchTimeMs = now;

    // ---- 计算最终得分：基础10分 + 连击加成（每级+5）----
    int baseScore = 10;                // 基础分：每次消除至少得10分
    int bonus = m_comboCount * 5;      // 加成：连击数乘以5（连击1→5分，连击2→10分...）
    return baseScore + bonus;          // 返回总分
}

// ============================================================================
// findHint() —— 找一对可以消除的方块（"提示"按钮会用到它）
//
// 聪明的做法：先把所有方块按图案类型分组，然后只在同类型内部找配对。
// 比如80个方块分20组每组4个，每组只要检查6种配对 → 总共120次检查。
// 如果不用分组直接两两配对 → 要检查3160次。快了约26倍！
// ============================================================================
PathInfo GameBoard::findHint() const
{
    // ---- 创建"按类型分组"的数组 ----
    // positionsByType[1] = 所有类型1方块的位置列表
    // positionsByType[2] = 所有类型2方块的位置列表 ...
    // positionsByType[0] 不用（0=空格）
    QVector<QVector<QPoint>> positionsByType(m_tileTypes + 1);

    // ---- 遍历整个棋盘，收集每个方块的类型和位置 ----
    for (int r = 1; r <= m_rows; ++r)          // 遍历每一行（跳过边界行）
    {
        for (int c = 1; c <= m_cols; ++c)      // 遍历每一列（跳过边界列）
        {
            int type = m_grid[r][c];            // 取出这个格子的值（方块编号或0）
            if (type != EMPTY)                  // 如果这个格子不是空格（确实有方块）
                positionsByType[type].append(QPoint(c, r)); // 就把这个位置(列,行)加入对应类型的列表
        }
    }

    // ---- 对每种类型，尝试组内所有两两配对 ----
    for (int type = 1; type <= m_tileTypes; ++type) // 对于从1到tileTypes的每种类型
    {
        const QVector<QPoint>& positions = positionsByType[type]; // 获取这种类型的所有方块位置
        int count = positions.size();            // 这种类型还有多少个方块

        // 双重循环：外循环从0到count-2，内循环从外循环+1到count-1
        // 这样每个配对 (A,B) 只检查一次，不会出现 (A,B) 和 (B,A) 重复
        for (int i = 0; i < count; ++i)         // 对于列表中的第i个方块
        {
            for (int j = i + 1; j < count; ++j) // 对于第i+1到最后一个的每一个方块
            {
                int r1 = positions[i].y(), c1 = positions[i].x(); // 取第i个方块的行列
                int r2 = positions[j].y(), c2 = positions[j].x(); // 取第j个方块的行列
                PathInfo path = findPath(r1, c1, r2, c2); // 尝试找这两个方块的连接路径
                if (path.valid)                  // 如果找到有效路径
                    return path;                 // 就立即返回（找到一对就行了，不用找全部）
            }
        }
    }

    return PathInfo();  // 遍历了所有类型的所有配对都没找到——棋盘上已经没有可消除的配对了
}

// ============================================================================
// hasValidMoves() —— 棋盘上还有可以消除的配对吗？
// 直接复用 findHint() 的结果。如果 findHint 能找到 → 说明还有路可走。
// ============================================================================
bool GameBoard::hasValidMoves() const
{
    return findHint().valid;  // 如果 findHint() 返回的路径是有效的，就返回 true
}

// ============================================================================
// isWin() —— 判断是否通关了
// 通关条件：所有方块都被消除了（剩余方块数=0）
// ============================================================================
bool GameBoard::isWin() const
{
    return m_remainingTiles == 0;  // 如果剩余方块数为0，就返回true（通关！）
}

// ============================================================================
// shuffle() —— 重排棋盘（把剩余方块随机重新分配）
//
// 注意：只改变方块的"图案分布"，不改变"哪些位置有方块"。
// 空位还是空位，有方块的地方图案被重新洗牌。
//
// 玩家视角：画面一闪，所有方块的图案都变了，但空位位置不变。
// ============================================================================
void GameBoard::shuffle()
{
    // ---- 第1步：收集所有没有被消除的方块的类型编号 ----
    QVector<int> remaining;                    // 创建一个空列表来存剩余方块
    remaining.reserve(m_remainingTiles);       // 提前分配内存
    for (int r = 1; r <= m_rows; ++r)         // 遍历游戏区域每一行
    {
        for (int c = 1; c <= m_cols; ++c)     // 遍历每一列
        {
            if (m_grid[r][c] != EMPTY)         // 如果这个位置还有方块
                remaining.append(m_grid[r][c]); // 就把它的类型编号加入列表
        }
    }

    // ---- 第2步：用洗牌算法打乱列表的顺序 ----
    std::shuffle(remaining.begin(), remaining.end(), m_rng);

    // ---- 第3步：按打乱后的新顺序，把方块放回原位 ----
    int idx = 0;                               // 用 idx 指向 remaining 中下一个要放的方块
    for (int r = 1; r <= m_rows; ++r)         // 重新遍历棋盘游戏区域
    {
        for (int c = 1; c <= m_cols; ++c)
        {
            if (m_grid[r][c] != EMPTY)         // 如果这个位置应该有方块
                m_grid[r][c] = remaining[idx++]; // 把打乱后的新方块编号放进去，idx往后移
        }
    }
}

// ============================================================================
// serializeGrid() —— 把整个棋盘数据转成一维整数数组（存档用）
//
// 一维数组的格式：
//   [0~3]: 元数据（rows, cols, tileTypes, copiesPerType）
//   [4~]:  逐行逐列展平的棋盘数据
//
// 这个数组可以存到文件中（JSON），之后读取恢复棋盘。
// ============================================================================
QVector<int> GameBoard::serializeGrid() const
{
    int tr = totalRows();           // 含边界的总行数
    int tc = totalCols();           // 含边界的总列数
    QVector<int> data;              // 创建空的一维数组
    data.reserve(tr * tc + 4);     // 预分配足够的内存

    // ---- 写入元数据（恢复时用来知道棋盘尺寸） ----
    data.append(m_rows);            // 游戏行数
    data.append(m_cols);            // 游戏列数
    data.append(m_tileTypes);       // 图案种类数
    data.append(m_copiesPerType);   // 每种图案的副本数

    // ---- 逐行逐列把格子值写入数组 ----
    for (int r = 0; r < tr; ++r)            // 对于每一行（含边界行）
        for (int c = 0; c < tc; ++c)        // 对于每一列（含边界列）
            data.append(m_grid[r][c]);       // 把格子值（0=空格, 1~tileTypes=图案）加入数组

    return data;  // 返回展平后的一维数组
}

// ============================================================================
// deserializeGrid() —— 从一维数组恢复棋盘（存档读取时用）
//
// 步骤：先读出棋盘尺寸 → 重新分配二维数组 → 把数据填回去 →
// 重新计算剩余方块数。
// ============================================================================
void GameBoard::deserializeGrid(const QVector<int>& data)
{
    if (data.size() < 4) return;            // 如果数据太少（连元数据都不够），直接返回

    // ---- 读出元数据 ----
    m_rows = data[0];                       // 第0个是游戏行数
    m_cols = data[1];                       // 第1个是游戏列数
    m_tileTypes = data[2];                  // 第2个是图案种类数
    m_copiesPerType = data[3];              // 第3个是每种副本数

    int tr = totalRows();
    int tc = totalCols();

    // ---- 重新分配二维数组 ----
    m_grid.clear();                         // 清空旧的网格
    m_grid.resize(tr);                      // 分配 tr 行
    for (int r = 0; r < tr; ++r)           // 对于每一行
        m_grid[r].resize(tc);               // 分配 tc 列

    // ---- 把数据填入网格 ----
    int idx = 4;                            // 从第4个元素开始才是棋盘数据
    for (int r = 0; r < tr; ++r)           // 对于每一行
        for (int c = 0; c < tc; ++c)       // 对于每一列
            m_grid[r][c] = (idx < data.size()) ? data[idx++] : EMPTY;
            // 如果 idx 还没超出数组范围，就取 data[idx] 的值并 idx+1
            // 如果已经超出范围，设为零（空格）

    // ---- 重新计算还有多少个方块没消除 ----
    m_remainingTiles = 0;                   // 从0开始数
    for (int r = 1; r <= m_rows; ++r)      // 遍历游戏区域（不含边界）
        for (int c = 1; c <= m_cols; ++c)
            if (m_grid[r][c] != EMPTY)       // 如果这个位置有方块
                m_remainingTiles++;           // 计数加1
}
