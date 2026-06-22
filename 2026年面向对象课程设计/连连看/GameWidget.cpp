// ============================================================================
// 文件：GameWidget.cpp
// 角色："画布"——游戏画面如何显示、玩家如何交互，全部在此
//
// 本文件是连连看项目中最大、最复杂的文件。
// 每一行代码都配有"如果……就……"风格的自然语言注释。
//
// 先理解几个关键类的作用再读代码：
//   QPainter  = 画笔，所有画线、画矩形、画文字、画图片的操作都通过它完成
//   QTimer    = 定时器，到了指定时间就触发一次信号（类似闹钟）
//   QPixmap   = 保存图片的对象，加载 PNG 文件后存在显存里，绘制时直接用
//   QPoint    = 二维坐标(x,y)，用来表示棋盘格子的位置
// ============================================================================

#include "GameWidget.h"
#include <QPainter>        // QPainter —— Qt 的绘图工具类，相当于一支"画笔"
#include <QMouseEvent>     // QMouseEvent —— 鼠标事件的载体，包含鼠标的位置和按了什么键
#include <QResizeEvent>    // QResizeEvent —— 窗口大小变化事件的载体
#include <QtMath>          // QtMath —— 提供 qMin（取小值）、qMax（取大值）等数学运算
#include <QMessageBox>     // QMessageBox —— 消息弹窗类（本文件暂未直接用，但保留以备将来）

// ============================================================================
// 构造函数 —— 创建游戏画布对象时自动调用
// 它把画布的所有"初始状态"设置好：没选中、没动画、没提示、没连击……
// ============================================================================
GameWidget::GameWidget(int rows, int cols, int tileTypes, int copiesPerType,
                       int iconScale, QWidget* parent)
    : QWidget(parent)                // 首先，调用父类 QWidget 的构造函数
    , m_board(rows, cols, tileTypes, copiesPerType) // 然后，用传入的尺寸参数创建棋盘逻辑对象
    , m_iconScale(iconScale)         // 把图标缩放比例（如100=正常）存到成员变量
    , m_hasSelection(false)          // 一开始，玩家没有选中任何方块，所以设为 false
    , m_selectedRow(-1)              // 把选中行号设为 -1（-1表示"没有有效的选中"）
    , m_selectedCol(-1)              // 选中列号也设为 -1
    , m_isAnimating(false)           // 一开始没有消除动画在播放
    , m_showingHint(false)           // 一开始没有在显示提示
    , m_hintRow1(-1), m_hintCol1(-1) // 提示方块1的坐标初始化为 -1
    , m_hintRow2(-1), m_hintCol2(-1) // 提示方块2的坐标也初始化为 -1
    , m_isShuffling(false)           // 一开始没有在重排棋盘
    , m_showingShuffleMsg(false)     // 一开始没有在显示重排提示文字
    , m_shuffleMsgFrames(0)          // 重排提示剩余帧数=0
    , m_comboEffects()               // 连击特效列表初始为空（还没消过方块）
{
    // ---- 设置鼠标跟踪模式 ----
    // 如果设为 true → 鼠标悬停不动也会产生事件（大量 CPU 消耗）
    // 如果设为 false → 只有按下鼠标时才产生事件（省资源）
    // 连连看不需要"悬停"功能，所以设为 false
    setMouseTracking(false);

    // ---- 设置窗口最小尺寸 ----
    // 防止用户把窗口缩到看不见棋盘的程度
    // 500×420 是最小可用尺寸
    setMinimumSize(500, 420);

    // ===== 预加载方块图片到内存 =====
    // 如果在 paintEvent() 里每次从磁盘读取 PNG，游戏会非常卡
    // 启动时一次性加载到 m_tilePixmaps 列表里，后续绘制直接取用，速度快
    loadTileImages();

    // ===================================================================
    // 提示闪烁定时器 —— 让提示的绿色边框"一闪一闪的"
    //
    // 工作原理：
    //   定时器每 200 毫秒（0.2 秒）触发一次 → 闪烁次数+1 → 请求重绘
    //   → paintEvent() 看到闪烁次数是偶数时画绿色框，奇数时不画
    //   → 玩家看到绿色框一亮一灭 = "闪烁"
    //
    //   闪烁 12 次（约 2.4 秒）后自动停止——不能让提示一直闪
    // ===================================================================
    m_hintTimer = new QTimer(this);           // 创建提示闪烁定时器
    m_hintTimer->setInterval(200);            // 设置间隔为 200 毫秒（每秒闪 5 次）

    // connect() = 把定时器的"时间到了"信号和一段处理代码连接起来
    // QTimer::timeout = 定时器发出的信号（"我设定的时间到了！"）
    // [this]() = lambda 表达式，相当于一个匿名函数，this 让它可以访问成员变量
    connect(m_hintTimer, &QTimer::timeout, this, [this]() {
        m_hintFlashCount++;                    // 每当定时器触发，就把闪烁计数 +1
        if (m_hintFlashCount >= 12)            // 如果闪烁了 12 次（约2.4秒）
            clearHintTimer();                  // 就停止闪烁：清除提示标记、关定时器
        update();                              // 请求 Qt 重绘画面（下一帧会调用 paintEvent()）
    });

    // ===================================================================
    // 连击特效定时器 —— 控制"COMBO x3!"浮动文字的动画
    //
    // 为什么这个定时器和提示闪烁定时器分开？
    //   提示闪烁 200ms 一次就够了，连击文字需要流畅的飘动动画。
    //   如果合在一起 → 提示闪太快（难看），连击动太卡（不流畅）。
    //
    // 每 50ms 触发一次 ≈ 每秒 20 帧——人眼感觉连续且流畅
    // ===================================================================
    m_comboTimer = new QTimer(this);          // 创建连击特效定时器
    m_comboTimer->setInterval(50);            // 间隔 50 毫秒（每秒 20 帧）
    connect(m_comboTimer, &QTimer::timeout, this, [this]() {
        // ---- 处理连击特效的倒计时 ----
        // 从后往前遍历（因为如果某个特效到期要删除，从后往前删不会乱）
        for (int i = m_comboEffects.size() - 1; i >= 0; --i)
        {
            m_comboEffects[i].remainingFrames--;   // 这个特效的剩余帧数减1
            if (m_comboEffects[i].remainingFrames <= 0) // 如果剩余帧数用完了
                m_comboEffects.removeAt(i);        // 就把它从列表中删除
        }

        // ---- 处理重排提示的倒计时 ----
        if (m_showingShuffleMsg)                 // 如果正在显示"正在自动重排..."提示
        {
            m_shuffleMsgFrames--;                // 剩余帧数减1
            if (m_shuffleMsgFrames <= 0)         // 如果帧数用完了
            {
                m_showingShuffleMsg = false;     // 就隐藏提示文字
                m_isShuffling = false;           // 并且解除"重排中"的锁定
            }
        }

        update();                                // 请求重绘（每次计时器触发都要刷新画面）

        // 如果连击特效列表空了，并且也没有重排提示要显示
        if (m_comboEffects.isEmpty() && !m_showingShuffleMsg)
            m_comboTimer->stop();                // 就关掉这个定时器，省 CPU（它自己会在需要时重启）
    });

    // ===================================================================
    // 空闲自动提示定时器 —— 玩家 10 秒不操作就自动显示提示
    //
    // setSingleShot(true) = "只响一次"的闹钟
    //   当玩家操作时，调用 start() 重置 10 秒倒计时
    //   如果 10 秒内玩家没任何操作 → 定时器触发 → 自动显示提示
    // ===================================================================
    m_idleTimer = new QTimer(this);               // 创建空闲提示定时器
    m_idleTimer->setInterval(IDLE_HINT_DELAY);     // 设置间隔为 10000 毫秒（10秒）
    m_idleTimer->setSingleShot(true);              // 设为"一次性"模式（触发一次就停）

    connect(m_idleTimer, &QTimer::timeout, this, [this]() {
        // 定时器触发了！说明玩家 10 秒没操作
        // 但在显示提示之前，还要确认三个条件都满足：
        if (!m_isAnimating &&                  // 条件1：没有消除动画正在播放
            !m_board.isWin() &&                // 条件2：还没通关
            !m_showingHint)                    // 条件3：当前没在显示提示（避免重复触发）
            showHint();                        // 如果三个条件都满足，就自动显示提示
    });
}

// ============================================================================
// 析构函数 —— 对象销毁时自动调用
//
// 为什么里面什么都没有？
//   因为在构造函数中，所有 new 出来的对象（定时器、图片等）都传了 this 作为父对象。
//   Qt 的"父子对象树"机制会自动清理：父对象被销毁时，子对象也被自动销毁。
//   如果在这里手动 delete，反而会出错（double-free，重复释放内存）。
// ============================================================================
GameWidget::~GameWidget()
{
}

// ============================================================================
// loadTileImages() —— 把方块图片从资源文件加载到内存缓存中
//
// Qt 的资源系统：
//   .qrc 文件列出的图片在编译时会嵌入到 .exe 里面
//   访问路径以冒号开头，例如 ":/tiles/images/tile_01.png"
//   优点：不需要外部图片文件，exe 移到哪都能显示图
//
// 预加载（Preload）策略：
//   如果每次 paintEvent 才从磁盘/资源读 PNG → 每帧几十张图 → 卡到没法玩
//   启动时一次性加载全部 → 绘制时直接取 m_tilePixmaps[编号] → 飞快
// ============================================================================
void GameWidget::loadTileImages()
{
    m_tilePixmaps.clear();                       // 清空旧的图片缓存
    m_tilePixmaps.reserve(m_board.tileTypes());  // 预分配内存空间（避免反复扩容）

    int needed = m_board.tileTypes();            // 需要多少种不同的图案

    // 对于第 1 种到第 needed 种图案
    for (int i = 0; i < needed; ++i)
    {
        // 构造图片路径
        // QString(":/tiles/images/tile_%1.png").arg(i+1, 2, 10, QChar('0'))
        //   i+1 = 要格式化的数字（1,2,3,...）
        //   2   = 至少占2位（1→"01", 10→"10"）
        //   10  = 十进制
        //   QChar('0') = 位数不够时前面补0
        // 结果："tile_01.png" / "tile_02.png" / ...
        QString path = QString(":/tiles/images/tile_%1.png")
                           .arg(i + 1, 2, 10, QChar('0'));
        QPixmap pix(path);                       // 用这个路径加载图片
        if (!pix.isNull())                       // 如果加载成功（图片不为空）
        {
            m_tilePixmaps.append(pix);           // 就把这张图片加入缓存列表
        }
        else                                     // 否则（加载失败，一般不会发生）
        {
            // 生成一张纯色的方块作为备用图
            // QColor::fromHsv(色调, 饱和度, 亮度)
            // 色调 = i * 360 / needed → 让每种图案颜色均匀分布在整个色环上
            QPixmap fallback(64, 64);            // 创建 64×64 的空白图片
            fallback.fill(QColor::fromHsv(i * 360 / needed, 200, 255)); // 填满纯色
            m_tilePixmaps.append(fallback);      // 加入缓存
        }
    }
}

// ============================================================================
// setIconScale() —— 设置图标缩放百分比，然后立即重新布局和重绘
// 被 MainWindow 调用（玩家在设置里调了图标大小后）
// ============================================================================
void GameWidget::setIconScale(int percent)
{
    m_iconScale = percent;           // 把缩放比例（如80=小,100=正常,120=大）存起来
    computeLayout();                 // 根据新比例重新计算每个方块的像素大小
    update();                        // 请求重绘
}

// ============================================================================
// startNewGame() —— 开始一局全新的游戏（把所有状态"归零"）
// ============================================================================
void GameWidget::startNewGame()
{
    clearHintTimer();                // 如果旧游戏有提示在闪烁，先关掉

    m_board.reset();                 // 让棋盘逻辑对象重置（生成新的随机布局，分数/步数归零）
    m_hasSelection = false;          // 清除选中状态
    m_selectedRow = -1;              // 选中行号重置为 -1
    m_selectedCol = -1;              // 选中列号重置为 -1
    m_isAnimating = false;           // 清除动画状态
    m_animPath = PathInfo();         // 把动画路径重置为"无效"
    m_comboEffects.clear();          // 清除上一局残留的连击特效文字

    // 发送信号通知 MainWindow："状态变了，请更新状态栏！"
    emit scoreChanged(0);            // 分数变了 → 现在是 0
    emit tilesRemainingChanged(m_board.totalTiles()); // 剩余方块数变了
    emit moveCountChanged(0);        // 步数变了 → 现在是 0
    emit comboCountChanged(0);       // 连击数变了 → 现在是 0

    m_idleTimer->start();            // 启动空闲提示计时器（10 秒后自动提示）
    computeLayout();                 // 计算方块的像素大小和位置
    update();                        // 请求重绘
}

// ============================================================================
// showHint() —— 显示提示（高亮一对可以消除的方块）
//
// 两个触发来源：
//   1. 玩家点击"提示"按钮
//   2. 玩家 10 秒没操作，空闲定时器自动触发
//
// 流程：先看能不能找 → 找到就标绿 → 找不到就说明死局了，触发自动重排
// ============================================================================
void GameWidget::showHint()
{
    // ---- 前提条件检查：这些时候不能提示 ----
    if (m_isAnimating || m_isShuffling)          // 如果正在播放消除动画 或 正在重排
        return;                                   // 就什么也不做（别打扰）
    if (m_board.isWin())                          // 如果已经通关了
        return;                                   // 就不需要提示了

    // ---- 清除旧状态 ----
    clearHintTimer();                             // 如果之前有提示在闪，关掉
    m_hasSelection = false;                       // 如果玩家正在选方块，取消选择
    m_selectedRow = -1;
    m_selectedCol = -1;

    // ---- 查找可消除的配对 ----
    PathInfo hint = m_board.findHint();           // 问棋盘："有可以消除的方块吗？"

    if (!hint.valid)                              // 如果棋盘说"没了"（valid=false）
    {
        shuffleBoard();                           // 就自动重排（重新洗牌）
        return;                                   // 返回
    }

    // ---- 找到了配对！记录两个方块的位置，然后让它们闪起来 ----
    m_hintRow1 = hint.corners.first().y();        // 第一个提示方块的行号
    m_hintCol1 = hint.corners.first().x();        // 第一个提示方块的列号
    m_hintRow2 = hint.corners.last().y();         // 第二个提示方块的行号
    m_hintCol2 = hint.corners.last().x();         // 第二个提示方块的列号
    m_showingHint = true;                         // 标记：现在正在显示提示
    m_hintFlashCount = 0;                         // 闪烁次数从0开始
    m_hintTimer->start();                         // 启动闪烁定时器（开始一闪一闪！）

    m_idleTimer->stop();                          // 提示已经显示了，不需要再自动触发
    update();                                     // 请求重绘（让绿色高亮显示出来）
}

// ============================================================================
// shuffleBoard() —— 自动重排（棋盘死局时，自动洗牌）
//
// 设计细节：
//   不直接立即重排——先显示"正在自动重排..."文字约0.8秒让玩家看到提示，
//   然后才真正重排。这样玩家不会懵"咦方块怎么突然全变了？"
// ============================================================================
void GameWidget::shuffleBoard()
{
    if (m_isAnimating) return;                     // 如果正在消除动画中，就不重排

    clearHintTimer();                              // 关掉提示
    m_hasSelection = false;                        // 取消选中
    m_selectedRow = -1;
    m_selectedCol = -1;

    // ===== 第1阶段：锁定操作 + 显示提示文字 =====
    m_isShuffling = true;                          // 锁定：鼠标点击被忽略
    m_showingShuffleMsg = true;                    // 显示"正在自动重排..."
    m_shuffleMsgFrames = 16;                       // 显示 16 帧（16×50ms = 0.8秒）
    m_idleTimer->stop();                           // 重排期间不要触发空闲提示

    if (!m_comboTimer->isActive())                 // 如果连击计时器没在运行
        m_comboTimer->start();                     // 就启动它（驱动帧更新）

    emit noMovesLeft();                            // 通知 MainWindow 在状态栏显示消息
    update();                                      // 立即刷新画面

    // ===== 第2阶段：等 0.8 秒后，真正执行重排 =====
    // QTimer::singleShot(毫秒, 接收者, 槽函数) = "毫秒后自动调用这个函数一次"
    QTimer::singleShot(800, this, [this]() {       // 800 毫秒后
        m_board.shuffle();                         // 调用棋盘的重排方法——方块图案被洗牌
        m_showingShuffleMsg = false;               // 隐藏提示文字
        m_isShuffling = false;                     // 解锁操作
        m_idleTimer->start();                      // 重新开始 10 秒空闲计时
        update();                                  // 刷新画面
        checkGameState();                          // 再次检查（万一重排后还是死局？）
    });
}

// ============================================================================
// drawShuffleMessage() —— 画"正在自动重排..."的文字提示
//
// 显示效果：棋盘正中央，金色大字，半透明黑底 + 黑色描边，非常醒目
// ============================================================================
void GameWidget::drawShuffleMessage(QPainter& painter)
{
    // ---- 计算棋盘正中央的坐标 ----
    double x = m_offsetX + (m_board.totalCols() * m_tileSize) / 2.0;  // 水平中央
    double y = m_offsetY + (m_board.totalRows() * m_tileSize) / 2.0;  // 垂直中央

    // ---- 设置字体 ----
    QFont font;
    font.setPointSize(qMax(18, static_cast<int>(m_tileSize * 0.5))); // 字体大小：18号 或 半格格子大，取较大值
    font.setBold(true);                            // 加粗
    painter.setFont(font);                         // 把字体应用到画笔上

    QString text = QString::fromUtf8("无可用移动，正在自动重排...");

    // ---- 画半透明背景框（让文字在杂色背景上也清晰可见） ----
    QRectF textRect(x - 250, y - 25, 500, 50);    // 矩形区域：宽500高50，居中对齐
    painter.fillRect(textRect, QColor(0, 0, 0, 160)); // 用半透明黑色填充（alpha=160）

    // ---- 画黑色描边效果（向四个对角各偏移1像素画一遍） ----
    QPen outlinePen(QColor(0, 0, 0));              // 纯黑画笔
    painter.setPen(outlinePen);
    // 向四个方向偏移后各画一遍 → 形成"描边"效果
    for (int dx = -1; dx <= 1; dx += 2)            // dx = -1 然后 +1
        for (int dy = -1; dy <= 1; dy += 2)        // dy = -1 然后 +1
            painter.drawText(textRect.adjusted(dx, dy, dx, dy),
                             Qt::AlignCenter, text); // 在偏移后的位置画文字

    // ---- 画主体金色文字 ----
    painter.setPen(QPen(QColor("#FFD700")));        // 金色画笔
    painter.drawText(textRect, Qt::AlignCenter, text); // 在正中央位置画文字
}

// ============================================================================
// clearHintTimer() —— 停止提示闪烁，清除所有提示相关状态
// ============================================================================
void GameWidget::clearHintTimer()
{
    m_hintTimer->stop();         // 关掉提示闪烁定时器
    m_showingHint = false;       // 标记：不再显示提示
    m_hintRow1 = -1;             // 清空提示方块1的行号
    m_hintCol1 = -1;             // 清空提示方块1的列号
    m_hintRow2 = -1;             // 清空提示方块2的行号
    m_hintCol2 = -1;             // 清空提示方块2的列号
    m_hintFlashCount = 0;        // 闪烁计数器归零

    // 如果游戏还在进行，重新开始 10 秒空闲计时
    if (!m_board.isWin() && !m_isAnimating)  // 如果没通关 并且 没在动画中
        m_idleTimer->start();                // 就重新开始空闲计时
}

// ============================================================================
// deserializeBoard() —— 从存档数据恢复棋盘
// 被 MainWindow 在"继续上一次"时调用
// ============================================================================
void GameWidget::deserializeBoard(const QVector<int>& data)
{
    m_board.deserializeGrid(data);   // 把一维数组还原成二维棋盘网格
    loadTileImages();                // 重新加载图片（棋盘尺寸可能和之前不同）
    computeLayout();                 // 重新计算像素布局
}

// ============================================================================
// computeLayout() —— 计算每个方块在屏幕上多大、棋盘从哪里开始画
//
// 这叫"自适应布局"——不管窗口多大、棋盘多大、图标缩放多少，
// 都能算出合适的方块大小，让所有方块完整显示且居中。
//
// 算法步骤（4步）：
//   1. 根据可用空间算出最大能放多大的方块
//   2. 应用玩家设置的缩放比例
//   3. 如果缩放后太大超出窗口 → 缩回到刚好填满
//   4. 安全下限：方块最小不能低于 24 像素（太小点不中）
// ============================================================================
void GameWidget::computeLayout()
{
    double scale = m_iconScale / 100.0;            // 缩放比例（1.0=正常, 0.8=小, 1.2=大）
    double availW = width()  - 2.0 * MARGIN;       // 可用宽度 = 窗口宽度 - 两边留白
    double availH = height() - 2.0 * MARGIN;       // 可用高度 = 窗口高度 - 上下留白

    // ---- 第1步：计算不缩放时最大能放多大的方块 ----
    double baseTileW = availW / m_board.totalCols(); // 如果只用宽度分配，每个格子多宽
    double baseTileH = availH / m_board.totalRows(); // 如果只用高度分配，每个格子多高
    double baseTile = qMin(baseTileW, baseTileH);    // 取宽高中较小的值（保证不溢出）

    // ---- 第2步：乘以缩放比例 ----
    m_tileSize = baseTile * scale;                   // 应用缩放（放大或缩小）

    // ---- 第3步：检查缩放后是否超出了可用空间 ----
    double boardW = m_tileSize * m_board.totalCols(); // 缩放后棋盘总宽度
    double boardH = m_tileSize * m_board.totalRows(); // 缩放后棋盘总高度
    if (boardW > availW || boardH > availH)           // 如果棋盘超出了窗口
    {
        // 就缩回到刚好填满窗口的大小
        m_tileSize = qMin(availW / m_board.totalCols(),
                          availH / m_board.totalRows());
    }

    // ---- 第4步：安全下限（不能太小，否则鼠标点不中） ----
    constexpr double MIN_TILE = 24.0;                // 定义最小方块尺寸为 24 像素
    if (m_tileSize < MIN_TILE)                       // 如果方块比 24 像素还小
        m_tileSize = MIN_TILE;                       // 就强制设为 24 像素

    // ---- 计算棋盘左上角坐标（让棋盘在窗口中居中） ----
    m_offsetX = (width()  - m_tileSize * m_board.totalCols()) / 2.0; // 水平居中
    m_offsetY = (height() - m_tileSize * m_board.totalRows()) / 2.0; // 垂直居中
}

// ============================================================================
// tileRect() —— 把棋盘逻辑坐标 (row, col) 换算成屏幕上的像素矩形
// 返回的是 QRectF 对象，包含 (左上X, 左上Y, 宽度, 高度)
// ============================================================================
QRectF GameWidget::tileRect(int row, int col) const
{
    double x = m_offsetX + col * m_tileSize;         // 这一列的左边缘 X 坐标
    double y = m_offsetY + row * m_tileSize;         // 这一行的上边缘 Y 坐标
    return QRectF(x, y, m_tileSize, m_tileSize);     // 返回一个正方形矩形
}

// ============================================================================
// tileCenter() —— 计算某个格子的中心点像素坐标
// 用于画连接线的端点和拐角圆圈（需要指向格子的正中心）
// ============================================================================
QPointF GameWidget::tileCenter(int row, int col) const
{
    QRectF r = tileRect(row, col);                   // 先算出矩形区域
    return r.center();                               // 再取矩形的中心点
}

// ============================================================================
// hitTest() —— 判断玩家点击了什么位置
//
// 输入：鼠标在屏幕上的像素坐标
// 输出：对应的棋盘逻辑坐标（row, col），通过引用参数返回
// 返回值：0=点中了有效格子，-1=点中了无效位置（棋盘外/边界/非游戏区）
//
// 转换公式：行号 = (鼠标Y - 棋盘左上Y) / 方块大小，列号同理
// ============================================================================
int GameWidget::hitTest(const QPoint& pos, int& outRow, int& outCol) const
{
    // ---- 像素坐标 → 行列号 ----
    outCol = static_cast<int>((pos.x() - m_offsetX) / m_tileSize); // 列号
    outRow = static_cast<int>((pos.y() - m_offsetY) / m_tileSize); // 行号

    // ---- 检查是否在棋盘范围内 ----
    if (outRow < 0 || outRow >= m_board.totalRows() ||    // 如果行号超出范围
        outCol < 0 || outCol >= m_board.totalCols())      // 或者列号超出范围
        return -1;                                         // 就返回 -1（无效）

    // ---- 检查是否在游戏区域（不是边界行/列） ----
    if (outRow < 1 || outRow > m_board.rows() ||          // 如果行号在边界上
        outCol < 1 || outCol > m_board.cols())            // 或者列号在边界上
        return -1;                                         // 也返回 -1（边界不能放方块）

    return 0;                                              // 点中了有效游戏格子！
}

// ============================================================================
// paintEvent() —— 整个游戏画面的"总导演"
//
// 每当需要刷新画面时，Qt 会调用这个函数。
// 绘制顺序 = 图层的堆叠顺序（先画的在底层，后画的在上层）：
//   1.背景 → 2.方块 → 3.选中高亮 → 4.提示高亮 → 5.连线动画 → 6.连击文字 → 7.重排文字
// ============================================================================
void GameWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);                                // 创建画笔对象
    painter.setRenderHint(QPainter::Antialiasing, true);   // 开启反锯齿（线条更平滑）

    // ===== 第1层：画背景色和网格线 =====
    drawBackground(painter);

    // ===== 第2层：画所有没被消除的方块 =====
    for (int r = 1; r <= m_board.rows(); ++r)              // 对于每一行（跳过边界行）
    {
        for (int c = 1; c <= m_board.cols(); ++c)          // 对于每一列（跳过边界列）
        {
            int type = m_board.getTile(r, c);              // 获取这个格子的值（0=空，1~N=图案编号）
            if (type != GameBoard::EMPTY)                  // 如果这个位置不是空格（有方块）
                drawTile(painter, r, c, type);             // 就画这个方块
        }
    }

    // ===== 第3层：如果玩家选中了一个方块，画金色高亮边框 =====
    if (m_hasSelection && !m_isAnimating)                  // 如果有选中 并且 没在动画中
        drawSelection(painter, m_selectedRow, m_selectedCol); // 对选中的方块画金色边框

    // ===== 第4层：如果正在显示提示，画绿色高亮边框（偶数帧才画→闪烁效果） =====
    if (m_showingHint && m_hintFlashCount % 2 == 0)        // 如果正在提示 并且 闪烁次数是偶数
        drawHintHighlight(painter);                        // 就画绿色提示框

    // ===== 第5层：如果正在播放消除动画，画连接线段 =====
    if (m_isAnimating)                                     // 如果正在动画中
        drawConnectionPath(painter);                       // 就画彩色连接线 + 拐角圆圈

    // ===== 第6层：画连击特效文字（"COMBO x3!"飘起来～） =====
    drawComboEffects(painter);

    // ===== 第7层：如果正在重排，画重排提示文字 =====
    if (m_showingShuffleMsg)                               // 如果显示重排消息标志为 true
        drawShuffleMessage(painter);                       // 就画"正在自动重排..."
}

// ============================================================================
// drawBackground() —— 画背景
// 三层颜色：整个窗口深蓝灰 → 棋盘区域稍亮 → 网格线更浅
// ============================================================================
void GameWidget::drawBackground(QPainter& painter)
{
    // ---- 填充整个窗口 ----
    painter.fillRect(rect(), QColor("#2C3E50"));             // 用深蓝灰色涂满整个窗口

    // ---- 填充棋盘区域（比窗口四周各小一圈 MARGIN） ----
    QRectF boardRect(m_offsetX, m_offsetY,
                     m_tileSize * m_board.totalCols(),
                     m_tileSize * m_board.totalRows());
    painter.fillRect(boardRect, QColor("#34495E"));          // 棋盘区域用稍亮的蓝灰色

    // ---- 画网格线 ----
    painter.setPen(QPen(QColor("#3D566E"), 0.5));            // 设置细的浅色画笔

    for (int r = 0; r <= m_board.totalRows(); ++r)          // 画所有水平线
    {
        double y = m_offsetY + r * m_tileSize;               // 这条线的 Y 坐标
        painter.drawLine(QPointF(m_offsetX, y),
                         QPointF(m_offsetX + m_board.totalCols() * m_tileSize, y));
    }

    for (int c = 0; c <= m_board.totalCols(); ++c)          // 画所有竖直线
    {
        double x = m_offsetX + c * m_tileSize;               // 这条线的 X 坐标
        painter.drawLine(QPointF(x, m_offsetY),
                         QPointF(x, m_offsetY + m_board.totalRows() * m_tileSize));
    }
}

// ============================================================================
// drawTile() —— 画一个方块
// 根据类型编号从缓存中取对应的图片，缩放到格子的内缩区域
// ============================================================================
void GameWidget::drawTile(QPainter& painter, int row, int col, int type)
{
    int typeIndex = type - 1;                             // 图案编号 → 数组下标（比如编号1对应下标0）
    if (typeIndex < 0 || typeIndex >= m_tilePixmaps.size()) // 如果下标越界
        return;                                            // 就不画（安全处理）

    QRectF rect = tileRect(row, col);                      // 获取这个格子的像素矩形
    double padding = m_tileSize * 0.08;                    // 内缩 8%（让方块之间有小间隙）
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding); // 对矩形四个边各向内缩进

    const QPixmap& pix = m_tilePixmaps[typeIndex];         // 从缓存中取出这张图片
    painter.drawPixmap(inner.toRect(), pix);               // 缩放到内缩矩形大小并画上去
}

// ============================================================================
// drawSelection() —— 画玩家选中的方块的"金色发光高亮边框"
// 双层边框制造"发光"效果：内层细金边 + 外层粗半透明金边
// ============================================================================
void GameWidget::drawSelection(QPainter& painter, int row, int col)
{
    QRectF rect = tileRect(row, col);
    double padding = m_tileSize * 0.04;                    // 内缩 4% 作为高亮框的边距
    QRectF inner = rect.adjusted(padding, padding, -padding, -padding);

    // ---- 内层：深金色细线 ----
    QPen pen(QColor("#FFD700"), m_tileSize * 0.07);        // 创建金色画笔，线宽为格子的 7%
    pen.setJoinStyle(Qt::RoundJoin);                       // 拐角处用圆角（好看）
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);                         // 不填充内部，只画边框
    painter.drawRoundedRect(inner,                         // 画圆角矩形
                            m_tileSize * 0.12,             // 圆角的水平半径
                            m_tileSize * 0.12);            // 圆角的垂直半径

    // ---- 外层：浅金色半透明宽线（制造"光晕"效果） ----
    QPen outerPen(QColor(255, 215, 0, 100), m_tileSize * 0.12); // alpha=100→半透明
    painter.setPen(outerPen);
    painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);
}

// ============================================================================
// drawHintHighlight() —— 画提示高亮（绿色边框，两个方块同时画）
// 和选中高亮几乎一样，只是颜色改用绿色
// ============================================================================
void GameWidget::drawHintHighlight(QPainter& painter)
{
    QPen pen(QColor("#00FF88"), m_tileSize * 0.08);        // 绿色画笔，线宽为格子的 8%
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // 画两个提示方块（一个循环处理两个）
    for (int i = 0; i < 2; ++i)
    {
        int row = (i == 0) ? m_hintRow1 : m_hintRow2;      // 如果是第一次循环，取第一个方块；第二次取第二个
        int col = (i == 0) ? m_hintCol1 : m_hintCol2;
        QRectF rect = tileRect(row, col);
        double padding = m_tileSize * 0.04;
        QRectF inner = rect.adjusted(padding, padding, -padding, -padding);
        painter.drawRoundedRect(inner, m_tileSize * 0.12, m_tileSize * 0.12);
    }
}

// ============================================================================
// drawConnectionPath() —— 画消除路径的连接线和拐点圆圈
//
// 绘制四层效果（从外到内）：
//   1. 外发光黄线（粗、半透明）→ 制造光晕
//   2. 主体橙线（细、不透明）→ 真正的连线
//   3. 拐点橙色实心圆 → 标出转弯的位置
//   4. 端点白色实心圆 → 突出标记起点和终点
// ============================================================================
void GameWidget::drawConnectionPath(QPainter& painter)
{
    if (m_animPath.corners.size() < 2) return;             // 如果拐点不足2个，不画

    // ---- 把逻辑坐标拐点转为屏幕像素坐标 ----
    QVector<QPointF> pixelPoints;
    for (const QPoint& cp : m_animPath.corners)             // 对于路径中的每一个拐点
        pixelPoints.append(tileCenter(cp.y(), cp.x()));     // 取它的中心像素坐标加入列表

    // ---- 第1层：光晕（粗的半透明黄色线） ----
    painter.setPen(QPen(QColor(255, 200, 50, 80), m_tileSize * 0.14,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < pixelPoints.size() - 1; ++i)       // 对于每一段线段
        painter.drawLine(pixelPoints[i], pixelPoints[i + 1]); // 画一条线

    // ---- 第2层：主体连接线（橙色） ----
    painter.setPen(QPen(m_animColor, m_tileSize * 0.06,
                        Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (int i = 0; i < pixelPoints.size() - 1; ++i)
        painter.drawLine(pixelPoints[i], pixelPoints[i + 1]);

    // ---- 第3层：拐点圆圈（橙色填充，不画边） ----
    painter.setBrush(m_animColor);                         // 用橙色填充
    painter.setPen(Qt::NoPen);                             // 不要边框线
    for (int i = 1; i < pixelPoints.size() - 1; ++i)       // 中间的才是拐点（跳过首尾）
        painter.drawEllipse(pixelPoints[i],                // 圆心位置
                             m_tileSize * 0.08,            // 水平半径
                             m_tileSize * 0.08);           // 垂直半径

    // ---- 第4层：端点白色圆（起点和终点） ----
    painter.setBrush(QColor(255, 255, 255));               // 纯白填充
    painter.drawEllipse(pixelPoints.first(),               // 起点
                         m_tileSize * 0.1, m_tileSize * 0.1);
    painter.drawEllipse(pixelPoints.last(),                // 终点
                         m_tileSize * 0.1, m_tileSize * 0.1);
}

// ============================================================================
// drawComboEffects() —— 画所有连击特效文字
//
// "浮动文字"的效果：
//   消除方块时，在消除位置上方生成"COMBO x3!"文字
//   文字慢慢向上飘约2个格子距离，同时越来越透明，最后消失
//
// 透明度变化曲线：
//   0%-20%：渐入（从透明变不透明）
//   20%-70%：全显（清晰可见）
//   70%-100%：渐出（从不透明变透明消失）
// ============================================================================
void GameWidget::drawComboEffects(QPainter& painter)
{
    for (const ComboEffect& fx : m_comboEffects)            // 对于列表中每一个连击特效
    {
        if (fx.comboCount < 2) continue;                   // 如果连击 < 2（太普通），就不显示

        // ---- 计算动画进度：0=刚开始, 1=快消失了 ----
        float progress = 1.0f - fx.remainingFrames / float(COMBO_FLOAT_FRAMES);

        // ---- 根据进度决定透明度：前20%渐入，中间全显，后30%渐出 ----
        float alpha;
        if (progress < 0.2f)                               // 如果进度在 0~20%
            alpha = progress / 0.2f;                        // 透明度从 0 递增到 1
        else if (progress > 0.7f)                           // 如果进度在 70~100%
            alpha = (1.0f - progress) / 0.3f;               // 透明度从 1 递减到 0
        else                                               // 否则（进度在 20~70% 之间）
            alpha = 1.0f;                                   // 保持完全不透明

        // ---- 竖直偏移：文字随时间向上飘 ----
        float yOffset = -progress * m_tileSize * 2.0f;     // 向上偏移 0 到 2 个格子高
        QPointF pos(fx.startPos.x(), fx.startPos.y() + yOffset);

        // ---- 根据连击等级选择不同颜色 ----
        QColor color;
        if (fx.comboCount >= 7)                             // 如果连击 7+
            // 每 3 帧切换一次颜色 → 金/红闪烁 → 非常有冲击力
            color = (fx.remainingFrames % 3 == 0) ? QColor("#FFD700") : QColor("#FF4444");
        else if (fx.comboCount >= 4)                        // 如果连击 4~6
            color = QColor("#FF69B4");                      // 粉红色
        else                                               // 连击 2~3
            color = QColor("#FF8C00");                      // 橙色

        color.setAlphaF(alpha);                             // 设置颜色的透明度

        QString text = QString("COMBO x%1!").arg(fx.comboCount); // 构造文字："COMBO x连击数!"

        // ---- 设置字体 ----
        QFont font;
        font.setPointSize(qMax(12, static_cast<int>(m_tileSize * 0.3)));
        font.setBold(true);
        painter.setFont(font);

        // ---- 先画黑色描边（让文字在任何背景上都可见） ----
        QPen outlinePen(QColor(0, 0, 0, static_cast<int>(180 * alpha))); // 黑色，透明度随文字变化
        painter.setPen(outlinePen);
        QRectF textRect(pos.x() - 120, pos.y() - 20, 240, 40); // 文字区域：240×40
        // 向四个对角偏移 1 像素各画一遍 → 形成描边
        painter.drawText(textRect.adjusted(-1, -1, -1, -1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted( 1, -1,  1, -1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted(-1,  1, -1,  1), Qt::AlignCenter, text);
        painter.drawText(textRect.adjusted( 1,  1,  1,  1), Qt::AlignCenter, text);

        // ---- 再画主体彩色文字 ----
        QPen textPen(color);
        painter.setPen(textPen);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

// ============================================================================
// mousePressEvent() —— 当玩家点击鼠标时，Qt 调用这个函数
//
// 这是交互逻辑的核心！处理流程分三种情况：
//
//   A. 点击无效位置（棋盘外 / 边界 / 空格）
//      → 取消所有选中，停止提示
//
//   B. 点击有效方块，但之前没选中
//      → 选中这个方块，用金色高亮标记
//
//   C. 点击有效方块，且之前有选中
//      → 如果点了同一个方块 → 取消选中
//      → 如果不同图案     → 切换选中到新方块
//      → 如果同图案       → 尝试配对消除！
// ============================================================================
void GameWidget::mousePressEvent(QMouseEvent* event)
{
    // ---- 正在动画或重排中 → 不接受任何点击 ----
    if (m_isAnimating || m_isShuffling) return;

    // ---- 判断玩家点到了什么位置 ----
    int row, col;
    if (hitTest(event->pos(), row, col) < 0)              // 如果点到了无效位置
    {
        m_hasSelection = false;                            // 就取消之前的所有选中
        m_selectedRow = -1;
        m_selectedCol = -1;
        clearHintTimer();                                  // 假设玩家不想看提示了，关掉
        update();                                          // 重绘
        return;                                            // 结束
    }

    // ---- 如果点到了空格（那里没有方块） ----
    if (m_board.isEmpty(row, col))                         // 如果这个位置是空格
    {
        m_hasSelection = false;                            // 取消选中
        m_selectedRow = -1;
        m_selectedCol = -1;
        clearHintTimer();                                  // 关掉提示
        update();
        return;
    }

    // ---- 点到了有效方块！ ----
    clearHintTimer();                                      // 玩家在手动操作，关掉提示

    // 重置空闲计时：玩家有操作 → 10 秒倒计时重新开始
    m_idleTimer->start();

    if (!m_hasSelection)                                   // 如果之前没有选中任何方块
    {
        // ==== 情况B：第一次选中方块 ====
        m_hasSelection = true;                             // 标记为"有选中"
        m_selectedRow = row;                               // 记录选中的行号
        m_selectedCol = col;                               // 记录选中的列号
        update();                                          // 重绘（会显示金色高亮）
    }
    else                                                   // 否则（之前有选中）
    {
        // ==== 情况C：点了第二个方块 ====

        // ---- 如果点了同一个方块 → 取消选中（相当于"收回手"） ----
        if (m_selectedRow == row && m_selectedCol == col)  // 如果和上次选中的是同一个位置
        {
            m_hasSelection = false;                        // 取消选中
            m_selectedRow = -1;
            m_selectedCol = -1;
            update();
            return;
        }

        // ---- 如果两个方块图案不同 → 切换选中到新方块 ----
        if (m_board.getTile(row, col) !=                   // 如果新方块的类型
            m_board.getTile(m_selectedRow, m_selectedCol)) // 不等于旧方块的类型
        {
            m_selectedRow = row;                           // 选中切换到新方块
            m_selectedCol = col;
            update();
            return;
        }

        // ---- 图案相同 → 尝试配对消除！ ----
        tryMatch(row, col);
    }
}

// ============================================================================
// tryMatch() —— 尝试让两个选中的方块配对消除
// ============================================================================
void GameWidget::tryMatch(int row, int col)
{
    // 问棋盘："这两个方块能连起来吗？"
    PathInfo path = m_board.findPath(m_selectedRow, m_selectedCol, row, col);

    if (path.valid)                                        // 如果棋盘说"可以连！"
    {
        executeMatch(path);                                // 就执行消除动画（画线+消除）
    }
    else                                                   // 否则（不能连）
    {
        m_selectedRow = row;                               // 放弃旧选择，选中新方块
        m_selectedCol = col;
        update();
    }
}

// ============================================================================
// executeMatch() —— 开始执行消除动画
//
// 动画流程：
//   1. 锁定界面（不接受新点击）
//   2. 记录路径 → 下一帧会画出彩色连接线
//   3. 等 500 毫秒 → 让玩家看清楚连接线
//   4. 然后调用 finishMatch() 真正消除两个方块
// ============================================================================
void GameWidget::executeMatch(const PathInfo& path)
{
    m_isAnimating = true;                                  // 锁定界面：动画期间不接受点击
    m_animPath = path;                                     // 记录路径（drawConnectionPath 会用它画线）
    m_animColor = QColor("#FF8C00");                       // 连接线颜色设为橙色
    m_hasSelection = false;                                // 清除选中状态
    m_selectedRow = -1;
    m_selectedCol = -1;

    m_idleTimer->stop();                                   // 匹配成功了，暂时不需要自动提示

    update();                                              // 立即画一帧（显示连接线）

    // 500 毫秒（半秒）后调用 finishMatch 真正消除
    QTimer::singleShot(500, this, &GameWidget::finishMatch);
}

// ============================================================================
// finishMatch() —— 消除动画结束，真正移除方块并计算分数
//
// 这个函数在 executeMatch() 设置的 500ms 定时器触发后被调用。
// 连线已经显示了半秒，玩家看到效果了，现在该"干活"了。
// ============================================================================
void GameWidget::finishMatch()
{
    if (!m_animPath.valid) return;                         // 安全检查（不应该发生，但防万一）

    // ---- 从路径中提取两个方块的坐标 ----
    int r1 = m_animPath.corners.first().y();               // 第一个方块的行号
    int c1 = m_animPath.corners.first().x();               // 第一个方块的列号
    int r2 = m_animPath.corners.last().y();                // 第二个方块的行号
    int c2 = m_animPath.corners.last().x();                // 第二个方块的列号

    // ===== 计算本次消除的得分（含连击加成） =====
    int comboScore = m_board.calculateComboScore();        // 计算连击分
    int comboCount = m_board.getComboCount();              // 获取消除前的连击数

    // ===== 从棋盘上移除这两个方块 =====
    m_board.removeTiles(r1, c1, r2, c2);                  // 把两个位置变成空格
    m_board.addScore(comboScore);                          // 加上本次得分

    // ===== 如果连击数 >= 1，在方块上方生成浮动文字特效 =====
    if (comboCount >= 1)                                   // 如果至少是第2次连击
    {
        QPointF p1 = tileCenter(r1, c1);                   // 方块1的中心像素坐标
        QPointF p2 = tileCenter(r2, c2);                   // 方块2的中心像素坐标
        QPointF midPos((p1.x() + p2.x()) / 2.0,           // 两个方块中心点的 X 中点
                       qMin(p1.y(), p2.y()));               // 取两个方块中较上的那个 Y（文字从上面开始飘）

        ComboEffect fx;                                    // 创建一个新的连击特效对象
        fx.comboCount = comboCount + 1;                    // 本次是第几次连击（+1因为comboCount是消除前的值）
        fx.remainingFrames = COMBO_FLOAT_FRAMES;           // 设置总帧数（动画持续时间）
        fx.startPos = midPos;                              // 设置文字起始位置
        m_comboEffects.append(fx);                         // 把特效加入活动列表

        if (!m_comboTimer->isActive())                     // 如果连击特效计时器没在跑
            m_comboTimer->start();                         // 就启动它
    }

    // ===== 恢复非动画状态 =====
    m_isAnimating = false;                                 // 解除锁定
    m_animPath = PathInfo();                               // 清空路径记录

    // ===== 发信号通知 MainWindow 更新状态栏 =====
    emit scoreChanged(m_board.getScore());                 // "分数变了！"
    emit tilesRemainingChanged(m_board.getRemainingTiles());// "剩余方块数变了！"
    emit moveCountChanged(m_board.getMoves());             // "步数变了！"
    emit comboCountChanged(comboCount);                    // "连击数变了！"

    update();                                              // 重绘（被消除的方块消失）
    checkGameState();                                      // 检查：通关了？还是死局了？
}

// ============================================================================
// checkGameState() —— 消除一对后，检查游戏处于什么状态
//
// 三种可能：
//   A. 通关了（所有方块都消完了）→ 停止计时，发 gameWon 信号
//   B. 死局了（还有方块但都配不上对）→ 自动重排（最多洗10次牌）
//   C. 正常继续 → 重新开始空闲计时
// ============================================================================
void GameWidget::checkGameState()
{
    // ---- 情况A：通关了 ----
    if (m_board.isWin())                                   // 如果剩余方块数 == 0
    {
        m_idleTimer->stop();                               // 停止空闲计时（通关了不用再提示）
        emit gameWon();                                    // 发"通关了！"信号给 MainWindow
        return;
    }

    // ---- 情况B：死局（没有可消除的配对） ----
    if (!m_board.hasValidMoves())                          // 如果棋盘说"没有可配对的"
    {
        int maxAttempts = 10;                              // 最多重排 10 次
        while (!m_board.hasValidMoves() && maxAttempts-- > 0) // 当棋盘还是死局 且 还有尝试次数
        {
            m_board.shuffle();                             // 重排一次
        }
        update();                                          // 重绘新布局
        emit noMovesLeft();                                // 通知 MainWindow：死局了（MainWindow 会显示状态栏消息）
    }

    // ---- 不管哪种情况，重新开始空闲计时 ----
    m_idleTimer->start();
}

// ============================================================================
// resizeEvent() —— 玩家拖拽窗口边缘改变大小时触发
// 窗口变大 → 方块也跟着变大；窗口变小 → 方块也跟着缩小
// ============================================================================
void GameWidget::resizeEvent(QResizeEvent*)
{
    computeLayout();  // 根据新的窗口大小重新计算方块的像素尺寸和位置
    update();         // 用新布局重新绘制
}
