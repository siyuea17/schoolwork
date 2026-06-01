#pragma once
// ============================================================================
// 文件：MainWindow.h
// 角色：连连看的主窗口——管理菜单栏、工具栏、状态栏和游戏计时
//
// MainWindow 继承自 QMainWindow，这是 Qt 提供的"标准应用程序窗口"。
// QMainWindow 自带以下布局区域（这就是为什么不用自己画菜单栏）：
//
//   ┌──────────────────────────────────┐
//   │  MenuBar（菜单栏）                │  ← 文件/编辑/帮助等下拉菜单
//   ├──────────────────────────────────┤
//   │  ToolBar（工具栏）                │  ← 按钮：新游戏 | 提示
//   ├──────────────────────────────────┤
//   │                                  │
//   │  CentralWidget（中央区域）        │  ← 这里放 GameWidget（游戏画布）
//   │                                  │
//   ├──────────────────────────────────┤
//   │  StatusBar（状态栏）              │  ← 分数 | 时间 | 剩余 | 步数
//   └──────────────────────────────────┘
//
// .ui 文件是什么？
//   MainWindow.ui 是一个 XML 文件，用 Qt Designer 可视化设计。
//   编译时 Qt 的 UIC 工具会把它生成 C++ 代码（ui_MainWindow.h）。
//   我们在代码中用 ui.xxx 访问设计器里创建的控件。
//   这是 Qt 的"UI 与逻辑分离"理念。
// ============================================================================

#include <QtWidgets/QMainWindow>  // Qt 标准主窗口类
#include <QLabel>                 // 标签控件（用于状态栏的文字显示）
#include <QTimer>                 // 定时器（用于游戏计时）
#include "ui_MainWindow.h"        // 编译生成的 UI 代码
                                   // 这个文件由 UIC 工具从 MainWindow.ui 自动生成，
                                   // 不用手动编辑，也不在项目文件夹中（在编译输出目录）

class GameWidget;  // 前向声明（告诉编译器"GameWidget 是一个类"）
                   // 不包含 GameWidget.h 可以减少编译依赖，提高编译速度

// ============================================================================
// MainWindow —— 游戏主窗口
//
// 职责：
//   1. 提供窗口框架（菜单、工具栏、状态栏）
//   2. 管理游戏计时器（每秒更新一次）
//   3. 中转用户操作（工具栏按钮 → 调用 GameWidget 的函数）
//   4. 显示通关弹窗
//
// 注意：MainWindow 不处理游戏逻辑（路径查找、匹配判断等），
//       也不处理绘制（画棋盘、画方块等）。
//       它只负责"窗口管理"和"信息显示"。
// ============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT  // Qt 元对象宏：有了它才能使用 signals/slots

public:
    // 构造函数
    // parent = 父窗口指针。对于主窗口来说通常是 nullptr（没有父窗口）。
    explicit MainWindow(QWidget* parent = nullptr);

    // 析构函数
    ~MainWindow();

private slots:
    // ======================== 槽函数（被信号触发）========================
    //
    // slots 关键字：声明这些都是"槽"——可以被信号触发的函数。
    // 和普通成员函数有什么区别？
    //   槽函数可以被 connect() 连接到一个信号上，
    //   当那个信号发出时，槽函数自动被调用。
    //   除此之外，槽函数也是普通函数，也可以直接调用。

    void onNewGame();
        // 用户点击"新游戏"按钮 → 重置游戏、计时归零

    void onHint();
        // 用户点击"提示"按钮 → 高亮一对可消除的方块

    void onTimerTick();
        // 游戏计时器每秒触发一次 → 更新时间显示
        // （例如 "时间: 1分23秒"）

    void onGameWon();
        // 收到 GameWidget 的 gameWon() 信号 → 停止计时、弹窗庆祝

    void onNoMovesLeft();
        // 收到 GameWidget 的 noMovesLeft() 信号 → 状态栏显示提示

private:
    // ======================== 数据成员 ========================

    Ui::MainWindowClass ui;
        // ui 对象：包含了 .ui 文件中创建的所有控件。
        // 通过 ui.newGameAction 访问"新游戏"菜单项，
        // 通过 ui.statusBar 访问状态栏等。
        // 这个对象由 setupUi() 初始化。

    GameWidget* m_gameWidget;
        // 游戏画布控件——整个窗口的核心！
        // 它被设置为 centralWidget（占据窗口中央区域）。
        // 指针类型，因为 QMainWindow 会自动管理其生命周期。

    // ---- 状态栏标签 ----
    QLabel* m_scoreLabel;
        // 显示分数的标签："分数: 100"
    QLabel* m_timerLabel;
        // 显示时间的标签："时间: 1分23秒"
    QLabel* m_remainingLabel;
        // 显示剩余方块数的标签："剩余: 42"
    QLabel* m_movesLabel;
        // 显示步数的标签："步数: 15"

    // ---- 游戏计时 ----
    QTimer* m_gameTimer;
        // 计时器：每1000毫秒（1秒）触发一次 timeout() 信号
    int m_elapsedSeconds;
        // 游戏已经进行了多少秒
};
