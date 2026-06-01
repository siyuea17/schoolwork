// ============================================================================
// 文件：GameWidget.cpp
// 角色：GameWidget 的实现——游戏的所有"看得见"的行为都在这里
//
// 这个文件实现了：
//   - 游戏的绘制（画棋盘、方块、高亮、连线动画）
//   - 用户的交互（鼠标点击选方块、匹配判断）
//   - 游戏辅助功能（提示、自动重排、空闲自动提示）
//
// 你会在本文件中看到大量 Qt 的绘图 API。Qt 的绘图系统叫做
// "QPainter 框架"，核心概念：
//   QPainter = 画家/画笔，所有绘制操作都通过它
//   QPaintEvent = 绘制请求，告诉程序"该重画了"
//   QPixmap   = 位图图片（加载 PNG/JPG 等）
//
// 绘制流程：Qt 检测到需要刷新 → 调用 paintEvent() → 我们用 QPainter 画东西
// ============================================================================

#include "GameWidget.h"   // 自己的头文件
#include <QPainter>        // Qt 绘图核心类
#include <QMouseEvent>     // 鼠标事件
#include <QResizeEvent>    // 窗口大小改变事件
#include <QtMath>          // Qt 数学函数（如 qMin 取最小值）
#include <QMessageBox>     // 弹窗对话框（实际未在此文件中使用）

// ============================================================================
// 构造函数 —— 初始化所有状态变量和定时器
//
// 构造函数做了什么：
//   1. 初始化所有成员变量（给它们合理的初始值）
//   2. 加载方块图片
//   3. 创建两个定时器：提示闪烁定时器 和 空闲自动提示定时器
//
// 关于初始化列表中的默认值：
//   - m_hasSelection(false) — 一开始没有选中任何方块
//   - m_selectedRow(-1) / m_selectedCol(-1) — (-1,-1) 约定为"无效坐标"
//   - m_isAnimating(false) — 没有动画在播放
//   - m_showingHint(false) — 没有在显示提示
// ============================================================================
GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent)                // 先调用父类 QWidget 的构造函数
    , m_hasSelection(false)          // 初始：没有选中方块
    , m_selectedRow(-1)              // -1 表示"未选中任何行"
    , m_selectedCol(-1)              // -1 表示"未选中任何列"
    , m_isAnimating(false)           // 初始：没有动画播放
    , m_showingHint(false)           // 初始：没有显示提示
    , m_hintRow1(-1), m_hintCol1(-1) // 提示坐标初始化为无效值
    , m_hintRow2(-1), m_hintCol2(-1)
    , m_hintFlashCount(0)            // 闪烁计数从0开始
{
    // setMouseTracking(false) 表示：
    //   只有按下鼠标时才追踪移动，平时不追踪 → 节省性能
    //   如果设为 true，鼠标在窗口上移动（不按）也会收到事件
    setMouseTracking(false);

    // setMinimumSize 设置窗口最小尺寸。用户不能把窗口缩得比这更小。
    // 这是为了确保棋盘至少有合理的显示空间。
    setMinimumSize(500, 420);

    // ----- 加载方块图片 -----
    loadTileImages();

    // ===== 提示闪烁定时器（周期性） =====
    // 这个定时器会让提示边框"一闪一闪"的，非常醒目。
    // setInterval(200) = 每 200 毫秒切换一次亮/灭（1秒闪烁5次）。
    // 定时器类型：周期性（不是 singleShot），会一直重复触发。
    m_hintTimer = new QTimer(this);
    m_hintTimer->setInterval(200);

    // connect —— 信号槽连接
    // 语法：connect(发送者, 信号, 接收者, 槽函数)
    // 翻译：当 m_hintTimer 发出 timeout() 信号时，执行后面的 lambda 函数
    //
    // lambda 表达式（C++11 引入）：
    //   [this]() { ... }
    //   [this] = 捕获列表，让 lambda 内部可以使用 this 指针
    //   ()     = 参数列表（这里没有参数）
    //   { ... } = 函数体
    connect(m_hintTimer, &QTimer::timeout, this, [this]() {
        m_hintFlashCount++;            // 闪烁次数+1
        if (m_hintFlashCount >= 12)    // 闪烁12次（约2.4秒）后自动停止
            clearHintTimer();          // 停止闪烁
        update();                      // 触发重绘（会调用 paintEvent）
    });

    // ===== 空闲自动提示定时器（单发） =====
    // 这个定时器只触发一次：玩家10秒不操作，自动显示提示。
    // setSingleShot(true) = 设置单发模式（触发一次后自动停止）。
    // setInterval(IDLE_HINT_DELAY) = 10000毫秒 = 10秒。
    m_idleTimer = new QTimer(this);
    m_idleTimer->setInterval(IDLE_HINT_DELAY);
    m_idleTimer->setSingleShot(true);

    connect(m_idleTimer, &QTimer::timeout, this, [this]() {
        // 定时器触发时，检查三个条件：
        //   1. 没在播放动画（动画期间不打断）
        //   2. 没通关（通关了不需要提示）
        //   3. 没在显示提示（避免重复触发）
        if (!m_isAnimating && !m_board.isWin() && !m_showingHint)
            showHint();                // 自动显示提示！
    });
}

// ============================================================================
// 析构函数
// 由于所有 QObject 子对象（定时器）都通过 parent 机制管理，
// Qt 会自动删除它们，所以这里不需要手动 delete。
// ============================================================================
GameWidget::~GameWidget()
{
}

// ============================================================================
// loadTileImages() —— 从 Qt 资源系统加载方块图片
//
// Qt 资源系统（.qrc 文件）：
//   .qrc 是一个 XML 文件，列出了项目要用到的资源（图片、字体等）。
//   编译时这些资源会被嵌入到可执行文件 .exe 中，不需要外部文件！
//   访问路径格式：":/tiles/images/tile_01.png"（以冒号开头表示资源内嵌）
//
// 为什么要"预加载"？
//   如果在 paintEvent 里每次绘制时才从磁盘/资源读取图片，
//   每秒钟要读几十次（因为重绘很频繁），游戏会卡顿。
//   所以一次性把20张图片加载到 m_tilePixmaps 缓存中，
//   绘制时直接用，速度快。
//
// 关于 QPixmap vs QImage：
//   QPixmap = 为屏幕显示优化的图片（存在显存中），绘制快
//   QImage  = 为像素操作优化的图片（存在内存中），直接操作像素快
//   这里只需要显示，所以用 QPixmap。
// ============================================================================
void GameWidget::loadTileImages()
{
    m_tilePixmaps.clear();                         // 清空缓存
    m_tilePixmaps.reserve(GameBoard::TILE_TYPES); // 预分配20个位置

    for (int i = 0; i < GameBoard::TILE_TYPES; ++i)
    {
        // 构造图片路径
        // QString::arg(i + 1, 2, 10, QChar('0')) 的含义：
        //   i+1       = 要格式化的数字（1到20）
        //   2         = 最少占2位宽度
        //   10        = 十进制
        //   QChar('0')= 位数不够时用'0'填充
        // 结果："1"→"01", "9"→"09", "10"→"10", "20"→"20"
        QString path = QString(":/tiles/images/tile_%1.png")
                           .arg(i + 1, 2, 10, QChar('0'));

        QPixmap pix(path);                         // 加载图片
        if (!pix.isNull())                         // 如果加载成功（不是空图）
            m_tilePixmaps.append(pix);             // 加入缓存
    }
}

// ============================================================================
// startNewGame() —— 开始新游戏
//
// 被 MainWindow 调用（用户点击"新游戏"按钮，或程序刚启动时）。
//
// 做了哪些清理工作：
//   1. 停止提示闪烁（清除旧游戏的提示状态）
//   2. 重置棋盘（生成新的随机布局）
//   3. 清除选择状态
//   4. 清除动画状态
//   5. 发送信号通知 MainWindow 更新状态栏
//   6. 启动空闲提示计时器
//   7. 重新计算布局并重绘
// ============================================================================
void GameWidget::startNewGame()
{
    clearHintTimer();          // 停止旧游戏的提示闪烁

    m_board.reset();           // 重置棋盘：新随机布局，分数/步数归零
    m_hasSelection = false;    // 清除选中
    m_selectedRow = -1;
    m_selectedCol = -1;
    m_isAnimating = false;     // 清除动画
    m_animPath = PathInfo();   // 重置为无效路径

    // 发送三个信号通知 MainWindow 更新状态栏
    emit scoreChanged(0);                        // 分数变0
    emit tilesRemainingChanged(GameBoard::TOTAL_TILES); // 剩余80个
    emit moveCountChanged(0);                    // 步数变0

    m_idleTimer->start();      // 启动空闲计时：10秒后自动提示
    computeLayout();           // 计算方块大小和位置
    update();                  // 触发重绘
    // update() 是 Qt 的"请求重绘"函数。
    // 它不会立即绘制，而是把请求放入事件队列，等当前处理完后再统一绘制。
    // 这可以避免短时间内多次重绘造成的性能浪费。
}

// ============================================================================
// showHint() —— 显示提示功能
//
// 被调用的情况：
//   1. 用户点击工具栏的"提示"按钮
//   2. 玩家空闲10秒自动触发
//
// 算法流程：
//   1. 检查前提条件（动画中？通关了？）→ 不满足就返回
//   2. 清除之前的提示和选择状态
//   3. 调用 m_board.findHint() 找可消除的配对
//   4. 找到了 → 设置提示高亮 → 启动闪烁计时器
//   5. 找不到 → 自动重排棋盘
// ============================================================================
void GameWidget::showHint()
{
    // ---- 前提条件检查 ----
    if (m_isAnimating) return;   // 正在播放消除动画，不提示
    if (m_board.isWin()) return; // 已经通关了，不需要提示

    // ---- 清除旧提示和选择 ----
    clearHintTimer();            // 停止之前的提示闪烁
    m_hasSelection = false;      // 取消选中状态（提示优先于手动选择）
    m_selectedRow = -1;
    m_selectedCol = -1;

    // ---- 查找可配对的方块 ----
    PathInfo hint = m_board.findHint();

    if (!hint.valid)
    {
        // 没找到！棋盘上虽然还有方块，但没有能配对的 → 死局
        // 解决方案：自动重排棋盘
        shuffleBoard();
        // 通知 MainWindow 在状态栏显示"正在自动重排"
        emit noMovesLeft();
        return;
    }

    // ---- 找到了！设置提示高亮 ----
    // PathInfo 的 corners 按顺序存储了路径拐点：
    //   corners.first() = 起点（第一个提示方块）
    //   corners.last()  = 终点（第二个提示方块）
    // QPoint 的 x=列, y=行
    m_hintRow1 = hint.corners.first().y();    // 第一个方块的行号
    m_hintCol1 = hint.corners.first().x();    // 第一个方块的列号
    m_hintRow2 = hint.corners.last().y();     // 第二个方块的行号
    m_hintCol2 = hint.corners.last().x();     // 第二个方块的列号
    m_showingHint = true;                     // 标记：正在显示提示
    m_hintFlashCount = 0;                     // 闪烁计数归零
    m_hintTimer->start();                     // 启动闪烁计时器

    // 提示已显示，不需要再自动提示了，停止空闲计时
    m_idleTimer->stop();

    update();  // 触发重绘
}

// ============================================================================
// shuffleBoard() —— 重排方块（仅内部调用）
//
// 这个函数是 private 的——外部（MainWindow）不能直接调用。
// 只有在下面两种情况下会自动调用：
//   1. 玩家请求提示，但棋盘上没有可消除的配对
//   2. 消除一对方块后，棋盘陷入死局（checkGameState 检测到）
//
// 重排后棋盘上的方块图案改变，但空位不变。
// ============================================================================
void GameWidget::shuffleBoard()
{
    if (m_isAnimating) return;   // 动画中不能重排

    clearHintTimer();            // 清除提示
    m_hasSelection = false;      // 清除选择
    m_selectedRow = -1;
    m_selectedCol = -1;

    m_board.shuffle();           // 调用 GameBoard 的重排算法

    // 重排后重新开始空闲计时，让玩家有10秒观察新棋盘
    m_idleTimer->start();
    update();                    // 重绘显示新布局
}

// ============================================================================
// clearHintTimer() —— 停止提示闪烁、清除提示状态
//
// 什么时候需要停止提示？
//   - 玩家自己点击了方块（开始手动操作）
//   - 提示闪烁了12次（约2.4秒）自己停
//   - 开始新游戏
//   - 开始消除动画
//
// 停止后如果游戏还在进行且没在动画中，重新启动空闲计时器。
// ============================================================================
void GameWidget::clearHintTimer()
{
    m_hintTimer->stop();         // 停止闪烁
    m_showingHint = false;       // 标记：不再显示提示
    m_hintRow1 = -1;             // 清除提示坐标
    m_hintCol1 = -1;
    m_hintRow2 = -1;
    m_hintCol2 = -1;
    m_hintFlashCount = 0;        // 闪烁计数归零

    // 如果游戏还在进行中（没通关、没在动画），重新启动空闲计时
    if (!m_board.isWin() && !m_isAnimating)
        m_idleTimer->start();    // 又开始10秒倒计时
}

// ============================================================================
// computeLayout() —— 计算方块的像素布局
//
// 这个函数根据当前窗口大小，计算：
//   - 每个方块的边长（像素）
//   - 棋盘左上角的起始坐标
//
// 算法思路：
//   1. 把窗口的可用空间（减去边距）除以 行数/列数，算出最大可能的方块尺寸
//   2. 取宽高中较小的那个作为实际方块尺寸（保证方块是正方形且完整显示）
//   3. 居中放置棋盘
// ============================================================================
void GameWidget::computeLayout()
{
    // 可用宽度 = 窗口宽度 - 左右边距
    double availW = width() - 2.0 * MARGIN;
    // 可用高度 = 窗口高度 - 上下边距
    double availH = height() - 2.0 * MARGIN;

    // 按宽度能放多大的方块？按高度能放多大的？
    double tileW = availW / GameBoard::TOTAL_COLS;   // 含边界的总列数12
    double tileH = availH / GameBoard::TOTAL_ROWS;   // 含边界的总行数10

    // 取较小的值，保证所有方块都能完整显示
    // qMin = Qt 版本的 min（和 std::min 类似，但某些平台表现更好）
    m_tileSize = qMin(tileW, tileH);

    // 计算偏移量使棋盘居中
    // 例如：窗口宽500，棋盘宽 50*12=600？不对，m_tileSize 已经取小了
    // 实际棋盘占用宽度 = m_tileSize * TOTAL_COLS
    // 偏移 = (窗口宽度 - 棋盘宽度) / 2
    m_offsetX = (width() - m_tileSize * GameBoard::TOTAL_COLS) / 2.0;
    m_offsetY = (height() - m_tileSize * GameBoard::TOTAL_ROWS) / 2.0;
}

// ============================================================================
// tileRect() —— 逻辑坐标转换为像素矩形
//
// 输入：(row, col) = 棋盘上的逻辑坐标（第几行第几列）
// 输出：QRectF = 这个格子占据的像素矩形区域
//
// QRectF 是什么？
//   一个表示矩形的类，存储左上角坐标 (x, y) 和宽高 (w, h)。
//   结尾的 F 表示使用 float/double 精度（非整数像素）。
// ============================================================================
QRectF GameWidget::tileRect(int row, int col) const
{
    double x = m_offsetX + col * m_tileSize;  // 左边缘 X 坐标
    double y = m_offsetY + row * m_tileSize;  // 上边缘 Y 坐标
    return QRectF(x, y, m_tileSize, m_tileSize);  // 宽高都是 m_tileSize（正方形）
}

// ============================================================================
// tileCenter() —— 计算某个格子的中心点
//
// 用于绘制连接线段和拐点圆圈，因为线的端点应该是方块的中心。
// ============================================================================
QPointF GameWidget::tileCenter(int row, int col) const
{
    QRectF r = tileRect(row, col);
    return r.center();  // QRectF 自带 center() 方法
}

// ============================================================================
// hitTest() —— 判断用户的点击位置对应哪个格子
//
// 输入：(pos, outRow, outCol)
//   pos    = 用户点击的像素坐标
//   outRow = 输出参数（引用传递），用于返回对应的行号
//   outCol = 输出参数（引用传递），用于返回对应的列号
//
// 返回值：
//    0  = 成功命中有效的游戏格子
//   -1  = 点击位置无效（在棋盘外、边界上、或空格）
//
// 算法：把像素坐标反向换算成行列号
//   行号 = (鼠标Y - 棋盘起始Y) / 方块大小
//   列号 = (鼠标X - 棋盘起始X) / 方块大小
//   如果行列号在合法范围 [1, ROWS/COLS] 内 → 有效；否则无效
// ============================================================================
int GameWidget::hitTest(const QPoint& pos, int& outRow, int& outCol) const
{
    // 像素坐标 → 逻辑行列号
    // static_cast<int>() = 显式类型转换（double → int，截断小数）
    outCol = static_cast<int>((pos.x() - m_offsetX) / m_tileSize);
    outRow = static_cast<int>((pos.y() - m_offsetY) / m_tileSize);

    // 检查是否在棋盘数组范围内（含边界）
    if (outRow < 0 || outRow >= GameBoard::TOTAL_ROWS ||
        outCol < 0 || outCol >= GameBoard::TOTAL_COLS)
        return -1;  // 完全不在棋盘上

    // 检查是否在游戏区域范围内（排除边界行/列）
    // 边界行是第0行和第9行（TOTAL_ROWS-1）
    // 边界列是第0列和第11列（TOTAL_COLS-1）
    if (outRow < 1 || outRow > GameBoard::ROWS ||
        outCol < 1 || outCol > GameBoard::COLS)
        return -1;  // 点在了边界上（不能放方块的地方）

    return 0;  // 点在了有效格子内！
}

// ============================================================================
// paintEvent() —— 绘制事件处理函数（整个游戏渲染的入口！）
//
// 这是 Qt 绘图系统最核心的函数。何时会被调用？
//   - 窗口第一次显示
//   - 调用 update() 后（比如方块被选中、动画帧更新）
//   - 窗口从最小化恢复或被其他窗口遮挡后重新露出
//   - 操作系统认为需要重绘
//
// 绘制顺序很重要（后画的会覆盖在先画的之上）：
//   1. 背景（深色底色 + 网格线）
//   2. 方块（按行列逐个绘制）
//   3. 选中高亮（如果有选中的方块）
//   4. 提示高亮（如果正在显示提示，且当前帧是亮帧）
//   5. 连接路径动画（如果正在播放消除动画）
// ============================================================================
void GameWidget::paintEvent(QPaintEvent* /*event*/)
{
    // QPainter 是 Qt 的"画笔"。
    // 创建时传入绘图设备（这里传 this，表示画在这个 widget 上）。
    QPainter painter(this);

    // setRenderHint = 设置渲染质量提示
    // Antialiasing = 反锯齿（让斜线和曲线边缘更平滑，不出现"锯齿"）
    // 代价是轻微的性能开销，但现代电脑完全没问题
    painter.setRenderHint(QPainter::Antialiasing, true);

    // ---- 第1层：画背景 ----
    drawBackground(painter);

    // ---- 第2层：画所有方块 ----
    // 遍历游戏区域（第1行到第8行，第1列到第10列）
    for (int r = 1; r <= GameBoard::ROWS; ++r)
    {
        for (int c = 1; c <= GameBoard::COLS; ++c)
        {
            int type = m_board.getTile(r, c);  // 获取这个位置的方块类型
            if (type != GameBoard::EMPTY)       // 如果不是空格
                drawTile(painter, r, c, type);  // 画这个方块
        }
    }

    // ---- 第3层：画选中高亮 ----
    // 条件：用户已选中一个方块 且 没在播放动画
    if (m_hasSelection && !m_isAnimating)
        drawSelection(painter, m_selectedRow, m_selectedCol);

    // ---- 第4层：画提示高亮（闪烁）----
    // 只在"偶数次闪烁"时显示（一亮一灭交替）
    // m_hintFlashCount % 2 == 0 → 第0,2,4,...次 → 显示
    // m_hintFlashCount % 2 == 1 → 第1,3,5,...次 → 不显示
    if (m_showingHint && m_hintFlashCount % 2 == 0)
        drawHintHighlight(painter);

    // ---- 第5层：画连接路径动画 ----
    if (m_isAnimating)
        drawConnectionPath(painter);
}
// 提示：paintEvent 会被频繁调用（每次 update() 都会触发），
// 因此它里面的代码应该尽量高效。这也是为什么我们预加载了图片缓存。

// ============================================================================
// drawBackground() —— 绘制棋盘背景和网格线
//
// 配色方案（深色调，保护眼睛）：
//   - 窗口背景色：   #2C3E50（深蓝灰）
//   - 棋盘区域背景： #34495E（稍亮的蓝灰）
//   - 网格线颜色：   #3D566E（微微可见的分隔线）
// ============================================================================
void GameWidget::drawBackground(QPainter& painter)
{
    // fillRect = 填充矩形区域为纯色
    // rect() = 获取整个 widget 的矩形 (0, 0, width, height)
    painter.fillRect(rect(), QColor("#2C3E50"));  // 整个窗口涂成深蓝灰

    // 棋盘区域（比窗口略小，因为四周留了 MARGIN 边距）
    QRectF boardRect(m_offsetX, m_offsetY,
                     m_tileSize * GameBoard::TOTAL_COLS,
                     m_tileSize * GameBoard::TOTAL_ROWS);
    painter.fillRect(boardRect, QColor("#34495E"));  // 棋盘区域稍亮

    // 画网格线（让玩家能看到格子的边界）
    // QPen = 画笔，定义线的颜色和粗细
    painter.setPen(QPen(QColor("#3D566E"), 0.5));

    // 画横线：一行一行地画
    for (int r = 0; r <= GameBoard::TOTAL_ROWS; ++r)
    {
        double y = m_offsetY + r * m_tileSize;
        // drawLine(起点, 终点)：从棋盘左边缘到右边缘画一条水平线
        painter.drawLine(QPointF(m_offsetX, y),
                         QPointF(m_offsetX + GameBoard::TOTAL_COLS * m_tileSize, y));
    }

    // 画竖线：一列一列地画
    for (int c = 0; c <= GameBoard::TOTAL_COLS; ++c)
    {
        double x = m_offsetX + c * m_tileSize;
        // drawLine(起点, 终点)：从棋盘上边缘到下边缘画一条垂直线
        painter.drawLine(QPointF(x, m_offsetY),
                         QPointF(x, m_offsetY + GameBoard::TOTAL_ROWS * m_tileSize));
    }
}

// ============================================================================
// drawTile() —— 绘制单个方块
//
// 参数：
//   row, col — 方块的行列位置
//   type     — 方块类型编号（1~20），对应 tile_01.png ~ tile_20.png
//
// 绘制策略：把 PNG 图片缩放后画到格子的内缩区域
//   inner = 格子向外留 padding 后的区域（稍微比格子小一点，更好看）
// ============================================================================
void GameWidget::drawTile(QPainter& painter, int row, int col, int type)
{
    // 方块类型 → 图片索引（type 从1开始，索引从0开始）
    int typeIndex = type - 1;
    if (typeIndex < 0 || typeIndex >= m_tilePixmaps.size()) return;

    QRectF rect = tileRect(row, col);

    // 内缩8%作为 padding，让方块之间有一点间距，更美观
    double padding = m_tileSize * 0.08;
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);
    // adjusted(dx1, dy1, dx2, dy2) = 四个边分别缩进
    //   左上角向内移 (padding, padding)
    //   右下角向内移 (-padding, -padding)
    //   所以 inner 比 rect 小一圈

    // drawPixmap = 把 QPixmap 画到指定矩形区域，自动缩放
    const QPixmap& pix = m_tilePixmaps[typeIndex];
    painter.drawPixmap(inner.toRect(), pix);
    // toRect() 把浮点矩形转为整数矩形（像素坐标必须是整数）
}

// ============================================================================
// drawSelection() —— 绘制选中方块的"高亮边框"
//
// 效果：两层金色圆角矩形边框
//   内层：深金色实线（#FFD700），较细
//   外层：淡金色半透明线，较粗，制造"发光"效果
//
// Qt 的画笔设置：
//   pen   = 线条的颜色、粗细、样式
//   brush = 填充颜色（NoBrush = 不填充，只画边框）
// ============================================================================
void GameWidget::drawSelection(QPainter& painter, int row, int col)
{
    QRectF rect = tileRect(row, col);
    double padding = m_tileSize * 0.04;       // 比方格内缩4%
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);

    // ---- 内层金边 ----
    // QPen(颜色, 线宽)
    // 线宽 = 方块大小的7%（随窗口大小动态调整）
    QPen pen(QColor("#FFD700"), m_tileSize * 0.07);
    pen.setJoinStyle(Qt::RoundJoin);           // 拐角处用圆角连接（更柔和）
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);             // 不填充，只画框
    painter.drawRoundedRect(inner,              // 矩形区域
                            m_tileSize * 0.12,  // 圆角X半径
                            m_tileSize * 0.12); // 圆角Y半径

    // ---- 外层发光 ----
    // 第4个参数 alpha=100（透明度，255=完全不透明，0=完全透明）
    QPen outerPen(QColor(255, 215, 0, 100), m_tileSize * 0.12);
    painter.setPen(outerPen);
    painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);
}

// ============================================================================
// drawHintHighlight() —— 绘制提示高亮（绿色边框）
//
// 和 drawSelection 类似，但用绿色且同时画两个方块。
// 只在闪烁的"亮帧"被调用（由 paintEvent 控制）。
// ============================================================================
void GameWidget::drawHintHighlight(QPainter& painter)
{
    // 绿色半透明边框（#00FF88），比选中框稍粗以增加醒目度
    QPen pen(QColor("#00FF88"), m_tileSize * 0.08);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // 画两个方块的提示框
    for (int i = 0; i < 2; ++i)
    {
        int row = (i == 0) ? m_hintRow1 : m_hintRow2;
        int col = (i == 0) ? m_hintCol1 : m_hintCol2;
        QRectF rect = tileRect(row, col);
        double padding = m_tileSize * 0.04;
        QRectF inner = rect.adjusted(padding, padding, -padding, -padding);
        painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);
    }
}

// ============================================================================
// drawConnectionPath() —— 绘制消除动画的连接路径
//
// 这个函数的视觉效果（从外到内共4层）：
//   1. 外发光：粗的半透明黄线（模拟光晕）
//   2. 主连接线：较细的橙色实线
//   3. 拐点圆圈：橙色填充圆（标出转弯的位置）
//   4. 端点白色圆：起点和终点各一个白圆（突出标记）
//
// 路径数据来源：m_animPath.corners
//   - 2个点 = 0折（直线），只有起点终点
//   - 3个点 = 1折（L形），起点→拐角→终点
//   - 4个点 = 2折（Z形），起点→拐角1→拐角2→终点
// ============================================================================
void GameWidget::drawConnectionPath(QPainter& painter)
{
    if (m_animPath.corners.size() < 2) return;  // 至少需要起点+终点

    // 把所有拐点的逻辑坐标转换为像素坐标
    QVector<QPointF> pixelPoints;
    for (const QPoint& cp : m_animPath.corners)
        pixelPoints.append(tileCenter(cp.y(), cp.x()));
        // 注意：cp 存储为 (x=col, y=row)，tileCenter 参数是 (row, col)

    // ---- 第1层：外发光（粗半透明）----
    // alpha=80 很透明，制造光晕扩散的感觉
    painter.setPen(QPen(QColor(255, 200, 50, 80), m_tileSize * 0.14,
                        Qt::SolidLine,              // 实线
                        Qt::RoundCap,               // 线端点用圆形（更好看）
                        Qt::RoundJoin));             // 拐角用圆形连接
    for (int i = 0; i < pixelPoints.size() - 1; ++i)
        painter.drawLine(pixelPoints[i], pixelPoints[i + 1]);

    // ---- 第2层：主连接线 ----
    painter.setPen(QPen(m_animColor, m_tileSize * 0.06,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < pixelPoints.size() - 1; ++i)
        painter.drawLine(pixelPoints[i], pixelPoints[i + 1]);

    // ---- 第3层：拐点圆圈 ----
    // 除了起点和终点外，中间的点都是拐角点
    painter.setBrush(m_animColor);   // 填充颜色 = 橙色
    painter.setPen(Qt::NoPen);       // 不画边框
    for (int i = 1; i < pixelPoints.size() - 1; ++i)
        painter.drawEllipse(pixelPoints[i],           // 圆心
                            m_tileSize * 0.08,        // X半径
                            m_tileSize * 0.08);        // Y半径

    // ---- 第4层：端点白色圆 ----
    painter.setBrush(QColor(255, 255, 255));  // 白色填充
    // 起点白圆
    painter.drawEllipse(pixelPoints.first(), m_tileSize * 0.1, m_tileSize * 0.1);
    // 终点白圆
    painter.drawEllipse(pixelPoints.last(), m_tileSize * 0.1, m_tileSize * 0.1);
}

// ============================================================================
// mousePressEvent() —— 鼠标点击事件处理
//
// 这是玩家交互的核心！点击的完整流程：
//
//  情况A：点击了有效方块
//   ├─ 之前没有选中 → 选中这个方块（高亮它）
//   └─ 之前有选中
//      ├─ 点了同一个方块 → 取消选中
//      ├─ 点了不同类型的方块 → 切换到新方块
//      └─ 点了同类型的方块 → 尝试匹配！
//         ├─ 可以连接 → 播放消除动画
//         └─ 不能连接 → 切换到新方块
//
//  情况B：点击了无效位置（棋盘外、边界、空格）
//   └─ 取消所有选中，停止提示
//
//  每次有效点击后都重置空闲计时器（说明玩家在操作）
// ============================================================================
void GameWidget::mousePressEvent(QMouseEvent* event)
{
    // ---- 动画中不接受点击 ----
    if (m_isAnimating) return;

    // ---- 判断点击位置 ----
    int row, col;
    if (hitTest(event->pos(), row, col) < 0)
    {
        // 点击无效：棋盘外/边界/非游戏区域
        m_hasSelection = false;
        m_selectedRow = -1;
        m_selectedCol = -1;
        clearHintTimer();        // 假设玩家不想看提示了
        update();
        return;
    }

    // ---- 点击了空格：取消选中 ----
    if (m_board.isEmpty(row, col))
    {
        m_hasSelection = false;
        m_selectedRow = -1;
        m_selectedCol = -1;
        clearHintTimer();
        update();
        return;
    }

    // ---- 点击了有效方块 ----
    clearHintTimer();  // 玩家在手动操作，关掉提示

    // 玩家有操作，重置空闲计时器（10秒倒计时重新开始）
    m_idleTimer->start();

    if (!m_hasSelection)
    {
        // ===== 之前没有选中 =====
        // 这是玩家点击的第一个方块 → 选中它
        m_hasSelection = true;
        m_selectedRow = row;
        m_selectedCol = col;
        update();  // 触发重绘（会显示金色高亮）
    }
    else
    {
        // ===== 之前有选中 =====
        // 这是玩家点击的第二个方块

        // 如果点了同一个方块 → 取消选中
        if (m_selectedRow == row && m_selectedCol == col)
        {
            m_hasSelection = false;
            m_selectedRow = -1;
            m_selectedCol = -1;
            update();
            return;
        }

        // 如果类型不同 → 切换到新选中的方块
        if (m_board.getTile(row, col) != m_board.getTile(m_selectedRow, m_selectedCol))
        {
            m_selectedRow = row;
            m_selectedCol = col;
            update();
            return;
        }

        // 类型相同！尝试匹配
        tryMatch(row, col);
    }
}

// ============================================================================
// tryMatch() —— 尝试匹配两个方块
//
// 用户选了两个同类型的方块后，调用 GameBoard::findPath() 查找路径。
//   找到了 → 执行消除动画
//   没找到 → 切换到新方块（取消旧选择，选中新方块）
// ============================================================================
void GameWidget::tryMatch(int row, int col)
{
    // 查找从旧选中到新点击的连接路径
    PathInfo path = m_board.findPath(m_selectedRow, m_selectedCol, row, col);

    if (path.valid)
    {
        // 找到了路径！执行消除动画
        executeMatch(path);
    }
    else
    {
        // 没找到路径 → 放弃旧选择，选中新方块
        m_selectedRow = row;
        m_selectedCol = col;
        update();
    }
}

// ============================================================================
// executeMatch() —— 执行消除动画
//
// 流程：
//   1. 设置动画状态（m_isAnimating = true）
//   2. 记录路径和颜色
//   3. 清除选择状态
//   4. 停止空闲计时（玩家刚操作了）
//   5. 绘制一帧（显示连接线）
//   6. 设置 500ms 后触发 finishMatch（500ms = 0.5秒）
//
// QTimer::singleShot(500, this, &GameWidget::finishMatch) 是什么意思？
//   "500毫秒后调用 this 的 finishMatch 函数，只调用一次。"
//   这是 Qt 提供的一种便捷定时器，不需要手动创建 QTimer 对象。
//   在这500毫秒期间，连接线保持显示，玩家可以看到消除效果。
// ============================================================================
void GameWidget::executeMatch(const PathInfo& path)
{
    m_isAnimating = true;               // 锁定界面（不接受点击）
    m_animPath = path;                  // 记录路径（paintEvent 会画它）
    m_animColor = QColor("#FF8C00");    // 橙色连接线
    m_hasSelection = false;             // 清除选择状态
    m_selectedRow = -1;
    m_selectedCol = -1;

    // 匹配成功，停止空闲计时（finishMatch 后会重新启动）
    m_idleTimer->stop();

    update();  // 立即画一帧（显示连接线）

    // 500毫秒后调用 finishMatch 真正消除方块
    // singleShot = 单次触发定时器，触发后自动销毁
    QTimer::singleShot(500, this, &GameWidget::finishMatch);
}

// ============================================================================
// finishMatch() —— 完成消除
//
// 消除动画结束后的收尾工作：
//   1. 从棋盘上真正移除两个方块
//   2. 加分
//   3. 恢复界面（停止动画）
//   4. 发送信号更新状态栏
//   5. 检查游戏状态（通关了？死局了？）
// ============================================================================
void GameWidget::finishMatch()
{
    if (!m_animPath.valid) return;  // 安全检查：路径必须是有效的

    // 提取两个方块的位置
    // corners.first() = 起点，corners.last() = 终点
    int r1 = m_animPath.corners.first().y();   // 第一个方块行号
    int c1 = m_animPath.corners.first().x();   // 第一个方块列号
    int r2 = m_animPath.corners.last().y();    // 第二个方块行号
    int c2 = m_animPath.corners.last().x();    // 第二个方块列号

    // 在棋盘数据上消除（把两个位置设为 EMPTY）
    m_board.removeTiles(r1, c1, r2, c2);
    m_board.addScore(10);  // 每消除一对加10分

    // 恢复非动画状态
    m_isAnimating = false;
    m_animPath = PathInfo();  // 清空路径

    // 发送信号通知 MainWindow 更新状态栏显示
    emit scoreChanged(m_board.getScore());
    emit tilesRemainingChanged(m_board.getRemainingTiles());
    emit moveCountChanged(m_board.getMoves());

    update();             // 重绘（移除已消除的方块）
    checkGameState();     // 检查：通关了？还是需要重排？
}

// ============================================================================
// checkGameState() —— 消除一对后检查游戏状态
//
// 每次消除完成后调用。两种结局：
//   A. 通关了 → 停止空闲计时，发送 gameWon 信号
//   B. 死局了 → 自动重排棋盘（最多尝试10次，通常1次就够）
//   C. 正常 → 重新开始空闲计时
// ============================================================================
void GameWidget::checkGameState()
{
    // ---- 检查是否通关 ----
    if (m_board.isWin())
    {
        m_idleTimer->stop();     // 通关了，不需要再自动提示
        emit gameWon();          // 通知 MainWindow 显示"恭喜通关"
        return;
    }

    // ---- 检查是否还有可消除的配对 ----
    if (!m_board.hasValidMoves())
    {
        // 死局！自动重排，循环直到有可用移动
        // 为什么不无限循环？加了 maxAttempts=10 做保险，
        // 极端情况下重排10次还是死局就放弃（几乎不会发生）
        int maxAttempts = 10;
        while (!m_board.hasValidMoves() && maxAttempts-- > 0)
        {
            m_board.shuffle();   // 打乱重试
        }
        update();               // 重绘显示新的随机布局
        emit noMovesLeft();     // 通知状态栏显示消息
    }

    // 重新开始空闲计时（无论是正常还是重排后）
    m_idleTimer->start();
}

// ============================================================================
// resizeEvent() —— 窗口大小改变事件
//
// 用户拖拽窗口边缘时触发。需要重新计算方块布局。
// ============================================================================
void GameWidget::resizeEvent(QResizeEvent* /*event*/)
{
    computeLayout();  // 重新计算方块的像素大小和位置
    update();         // 用新布局重新绘制
}
