#pragma once
// ============================================================================
// 文件：GameWidget.h
// 角色：连连看的"画布"——负责显示棋盘、处理点击、播放动画
//
// 这个类继承自 QWidget，是 Qt 中所有界面控件的基类。
//
// 为什么要把 GameWidget 单独写成一个类？
//   它承担了"控制器"的角色，连接 Model（GameBoard）和 View（屏幕显示）。
//   在软件架构中这叫做"关注点分离"：
//     - GameBoard（Model/模型）：只管数据和算法，不管怎么显示
//     - GameWidget（View+Controller/视图+控制）：管显示和交互，不懂算法细节
//     - MainWindow（Window/窗口）：管窗口框架（菜单、工具栏、状态栏）
//
// 继承关系（你用到的 Qt 类层次）：
//   QObject → QWidget → GameWidget
//     ↑          ↑           ↑
//   所有Qt    所有界面    你写的游戏画布
//   对象的    控件的
//   基类      基类
// ============================================================================

#include <QWidget>      // Qt 界面控件基类
#include <QTimer>        // Qt 定时器（可以定时触发事件）
#include <QVector>       // Qt 动态数组
#include <QPoint>        // Qt 点坐标
#include <QColor>        // Qt 颜色
#include <QPixmap>       // Qt 图片（离屏缓存的图像）
#include "GameBoard.h"   // 棋盘逻辑（GameWidget 内部包含一个 GameBoard）

// ============================================================================
// GameWidget —— 游戏区域控件
//
// 这是整个游戏最核心的界面类。它做的事情包括：
//   1. 画棋盘背景（深色网格）
//   2. 画每一个方块（从 .qrc 资源文件加载 PNG 图案）
//   3. 处理鼠标点击（选择方块、尝试匹配）
//   4. 画选中高亮（金色边框）
//   5. 画连接路径动画（彩色线段 + 拐点圆圈）
//   6. 画提示高亮（绿色闪烁边框）
//   7. 自动提示（10秒不操作自动显示提示）
//   8. 自动重排（无可用移动时自动打乱棋盘）
// ============================================================================
class GameWidget : public QWidget
{
    // Q_OBJECT 是 Qt 的一个宏，必须写在类定义的最前面。
    // 它的作用是让 Qt 的"元对象系统"识别这个类，
    // 有了它才能使用"信号和槽"（signals/slots）。
    // 简单理解：告诉 Qt "这个类需要信号槽功能，请帮我处理"。
    Q_OBJECT

public:
    // ======================== 构造与析构 ========================

    explicit GameWidget(QWidget* parent = nullptr);
    // explicit = 禁止隐式类型转换（C++ 关键字，一种良好的编程习惯）
    // parent 参数：Qt 的"父子对象树"机制。
    //   当你把一个 widget 的 parent 设为另一个 widget 时：
    //     - 子 widget 会显示在父 widget 内部
    //     - 父 widget 被删除时，会自动删除所有子 widget（不用手动 delete！）
    //   这是 Qt 内存管理的核心机制。

    ~GameWidget();  // 析构函数，对象销毁时调用

    // ======================== 游戏操作 ========================

    void startNewGame();  // 开始新游戏：重置棋盘、清除选择、随机生成方块
    void showHint();      // 显示提示：高亮一对可以消除的方块

    // ======================== 状态获取（给 MainWindow 查询用）========================

    int getScore() const { return m_board.getScore(); }
        // 获取当前分数

    int getMoves() const { return m_board.getMoves(); }
        // 获取当前步数

    // ======================== 信号（Signals）========================
    //
    // 什么是信号和槽（Signals & Slots）？
    //
    //   信号槽是 Qt 最核心的通信机制。你可以把它想象成"广播系统"：
    //     - 信号（signal）= 广播："我发生了一件事！"
    //     - 槽（slot）= 接收器："收到，我来处理这件事"
    //
    //   用 connect() 函数把信号和槽连起来后，
    //   只要发出信号，连接的槽函数就会自动被调用。
    //
    //   比如这里：
    //     GameWidget 发出 scoreChanged(100)
    //     → MainWindow 收到，更新状态栏显示"分数: 100"
    //
    //   signals 关键字：声明这是一个信号区（只声明，不需要实现！Qt 会自动生成代码）

signals:
    void scoreChanged(int newScore);
        // 分数变化信号。参数是新分数值。
        // MainWindow 接收后更新状态栏的分数显示。

    void tilesRemainingChanged(int remaining);
        // 剩余方块数变化信号。参数是剩余方块数量。
        // 用于更新状态栏"剩余: XX"的显示。

    void moveCountChanged(int moves);
        // 步数变化信号。参数是当前步数。
        // 用于更新状态栏"步数: XX"的显示。

    void gameWon();
        // 通关信号！所有方块都被消除了。
        // MainWindow 接收后停止计时，弹出"恭喜通关"对话框。

    void noMovesLeft();
        // 无可用移动信号。棋盘上还有方块但没有可配对的。
        // MainWindow 接收后在状态栏显示"正在自动重排..."

    // ======================== 事件处理（重写父类虚函数）========================
    //
    // Qt 是"事件驱动"的：程序大部分时间在等待用户操作，
    // 当用户做了某个操作（移动鼠标、点击、调整窗口大小），
    // Qt 会创建一个"事件"对象，然后调用对应的虚函数。
    //
    // 我们重写（override）这些虚函数，在父类默认行为的基础上加入自己的逻辑。
    //
    // 什么是虚函数（virtual function）？
    //   父类定义了一个函数，子类可以"重写"它。
    //   当你通过父类指针调用这个函数时，实际会执行子类的版本。
    //   这就是 C++ 的"多态"（polymorphism）。

protected:
    void paintEvent(QPaintEvent* event) override;
        // 绘制事件：需要重新画界面时 Qt 会调用这个函数。
        // 比如：窗口第一次显示、被其他窗口挡住后重新露出、调用 update() 后。
        // 这是整个游戏渲染的核心入口！

    void mousePressEvent(QMouseEvent* event) override;
        // 鼠标点击事件：用户在游戏区域按下鼠标时调用。
        // 在这里处理"选择方块"和"尝试配对"的逻辑。

    void resizeEvent(QResizeEvent* event) override;
        // 窗口大小改变事件：用户拖拽窗口边缘时调用。
        // 需要重新计算方块的布局（大小和位置）。

private:
    // ======================== 布局计算 ========================

    void computeLayout();
        // 计算方块的显示参数：每个方块多大、从哪里开始画。
        // 窗口大小改变时需要重新调用。

    // ======================== 绘制函数 ========================

    void drawBackground(QPainter& painter);
        // 画棋盘背景：深色填充 + 网格线

    void drawTile(QPainter& painter, int row, int col, int type);
        // 画一个方块：在指定位置画出指定类型（1~20）的图案
        // QPainter 是 Qt 的"画笔"，所有绘制操作都通过它完成

    void drawSelection(QPainter& painter, int row, int col);
        // 画选中高亮：金色圆角矩形边框（表示这个方块被选中了）

    void drawConnectionPath(QPainter& painter);
        // 画连接路径动画：彩色线段连接两个配对的方块，
        // 拐角处有圆圈，端点有白色圆点

    void drawHintHighlight(QPainter& painter);
        // 画提示高亮：绿色闪烁边框标记两个可配对的方块

    // ======================== 坐标转换 ========================

    QRectF tileRect(int row, int col) const;
        // 计算某个格子（逻辑坐标）在屏幕上的矩形区域（像素坐标）
        // 逻辑坐标 → 像素坐标的转换

    QPointF tileCenter(int row, int col) const;
        // 计算某个格子的中心点坐标（画连接线和圆圈时用）

    int hitTest(const QPoint& pos, int& outRow, int& outCol) const;
        // 点击测试：用户点击了屏幕上的像素位置，
        // 判断他点到了哪个逻辑格子（行号和列号）
        // 返回0表示点在了有效格子上，-1表示点到了棋盘外面或边界

    // ======================== 匹配逻辑 ========================

    void tryMatch(int row, int col);
        // 尝试匹配：用户选了第一个方块后又选了第二个，
        // 检查这两个能否消除，能消就执行消除动画

    void executeMatch(const PathInfo& path);
        // 执行消除动画：显示连接路径，0.5秒后真正消除方块

    void finishMatch();
        // 完成消除：消除动画结束后调用，真正从棋盘上移除方块

    void checkGameState();
        // 检查游戏状态：通关了？还是需要自动重排？
        // 每次消除完成后调用

    void shuffleBoard();
        // 自动重排：当没有可消除配对的时打乱方块。
        // 注意这个函数是 private 的——外部不能调用，
        // 只有 GameWidget 自己判断需要重排时才调用。

    // ======================== 资源加载 ========================

    void loadTileImages();
        // 从 Qt 资源文件（.qrc）加载20张方块图片到内存
        // 预加载到 QPixmap 缓存中可以加快绘制速度

    // ======================== 提示控制 ========================

    void clearHintTimer();
        // 清除提示状态：停止闪烁计时器，清除提示标记

    // ======================== 数据成员（状态变量）========================

private:
    // ---- 棋盘数据 ----
    GameBoard m_board;                      // 棋盘逻辑对象（组合关系）
                                            // GameWidget "拥有"一个 GameBoard

    // ---- 方块图片 ----
    QVector<QPixmap> m_tilePixmaps;         // 20张方块PNG图片的缓存
                                            // QPixmap 是 Qt 中用于显示的图片类型
                                            // 预先加载到内存中，绘制时直接用，
                                            // 不需要每次都从磁盘读取

    // ---- 布局参数（像素计算）----
    static constexpr int MARGIN = 15;       // 棋盘四周留白15像素
    double m_tileSize;                      // 每个方块的边长（像素）
    double m_offsetX;                       // 棋盘左上角的 X 坐标
    double m_offsetY;                       // 棋盘左上角的 Y 坐标
    // 注意：这些是 double（浮点数），因为窗口不一定能整除，
    // Qt 支持亚像素精度的绘制（反锯齿）

    // ---- 选择状态 ----
    bool m_hasSelection;                    // 用户是否已选中了一个方块？
    int m_selectedRow;                      // 已选中方块的行号（-1表示未选中）
    int m_selectedCol;                      // 已选中方块的列号

    // ---- 动画状态 ----
    bool m_isAnimating;                     // 是否正在播放消除动画？
                                            // 动画期间不接受点击，不触发提示
    PathInfo m_animPath;                    // 当前动画的连接路径
    QColor m_animColor;                     // 连接线的颜色（橙色）

    // ---- 提示状态 ----
    bool m_showingHint;                     // 是否正在显示提示？
    int m_hintRow1, m_hintCol1;            // 提示方块1的行列
    int m_hintRow2, m_hintCol2;            // 提示方块2的行列
    QTimer* m_hintTimer;                   // 提示闪烁计时器（每200毫秒切换闪烁）
    int m_hintFlashCount;                  // 已经闪烁了多少次（12次后停止）

    // ---- 空闲自动提示 ----
    QTimer* m_idleTimer;                   // 空闲计时器（单发模式）
    static constexpr int IDLE_HINT_DELAY = 10000;
        // 空闲多久触发自动提示（10000毫秒 = 10秒）
        // 玩家10秒不操作 → 自动显示提示
        // static constexpr = 编译时常量，不占对象内存，所有实例共享
};
