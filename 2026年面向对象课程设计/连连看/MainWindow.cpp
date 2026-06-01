// ============================================================================
// 文件：MainWindow.cpp
// 角色：MainWindow 类的实现——窗口创建、信号连接、游戏流程控制
//
// 这个文件演示了 Qt 应用程序的几个关键概念：
//   1. 信号槽连接（connect）：对象间通信的核心机制
//   2. Lambda 表达式：C++11 的匿名函数，大量用于槽函数
//   3. Qt 布局系统：QMainWindow 的各区域（状态栏、工具栏）
//   4. Qt 字符串：QString 和国际化处理（fromUtf8）
// ============================================================================

#include "MainWindow.h"    // 自己的头文件
#include "GameWidget.h"    // 游戏画布（中央控件）
#include <QMessageBox>      // 弹窗对话框（通关时用）
#include <QStatusBar>       // 状态栏
#include <QToolBar>         // 工具栏

// ============================================================================
// 构造函数 —— 创建整个窗口的所有内容
//
// 执行顺序（每一步都在为后面的步骤做准备）：
//   1. 初始化列表：所有指针设为 nullptr（空指针，避免野指针风险）
//   2. 加载 UI 文件（setupUi）
//   3. 创建游戏画布并设为中央控件
//   4. 连接工具栏按钮到处理函数
//   5. 创建状态栏标签（分数、时间、剩余、步数）
//   6. 创建游戏计时器
//   7. 连接 GameWidget 的信号到 MainWindow 的更新函数
//   8. 开始新游戏
// ============================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)           // 先调用父类 QMainWindow 的构造函数
    , m_gameWidget(nullptr)         // 指针初始化为空（稍后创建）
    , m_scoreLabel(nullptr)
    , m_timerLabel(nullptr)
    , m_remainingLabel(nullptr)
    , m_movesLabel(nullptr)
    , m_gameTimer(nullptr)
    , m_elapsedSeconds(0)           // 计时从0开始
{
    // ---- 第1步：加载 .ui 文件 ----
    // setupUi 是由 UIC 工具自动生成的函数（在 ui_MainWindow.h 中）。
    // 它会创建 .ui 文件中定义的所有控件（工具栏、状态栏、菜单等），
    // 并设置它们的属性（大小、标题、布局等）。
    ui.setupUi(this);

    // ---- 第2步：创建游戏画布 ----
    // new 在堆上分配内存，返回指向新对象的指针。
    // this 作为 parent 传入，意味着：
    //   - GameWidget 会显示在 MainWindow 内部
    //   - MainWindow 销毁时会自动 delete GameWidget（Qt 父子机制）
    m_gameWidget = new GameWidget(this);

    // setCentralWidget = 把这个控件放到主窗口的"中央区域"
    setCentralWidget(m_gameWidget);

    // ---- 第3步：连接工具栏按钮 ----
    // connect(谁发出信号, 什么信号, 谁接收, 调用哪个槽)
    //
    // QAction 是什么？
    //   "操作"的抽象。菜单项、工具栏按钮、快捷键都可以绑定到同一个 QAction。
    //   ui.newGameAction 是在 .ui 文件中定义的工具栏按钮。
    //
    // &MainWindow::onNewGame = C++ 成员函数指针
    //   指向 MainWindow 类的 onNewGame 函数
    connect(ui.newGameAction, &QAction::triggered,
            this, &MainWindow::onNewGame);
    connect(ui.hintAction, &QAction::triggered,
            this, &MainWindow::onHint);

    // ---- 第4步：创建状态栏标签 ----
    // QStatusBar 是 QMainWindow 自带的状态栏（底部横条）。
    // 我们用 addWidget 往里面添加 QLabel 来显示游戏信息。
    QStatusBar* sb = ui.statusBar;

    // --- 分数标签 ---
    // QString::fromUtf8("分数: 0")
    //   Qt 的字符串类 QString。fromUtf8 从 UTF-8 编码的中文文本创建字符串。
    //   如果直接用 "分数: 0"（char*），在中文 Windows 下可能乱码，
    //   所以 Qt 推荐用 fromUtf8 明确指定编码。
    m_scoreLabel = new QLabel(QString::fromUtf8("分数: 0"));
    m_scoreLabel->setMinimumWidth(120);  // 最小宽度120像素（防止文字被截断）
    // setStyleSheet = CSS 样式表（和网页 CSS 语法类似）
    m_scoreLabel->setStyleSheet("font-weight: bold; font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_scoreLabel);         // 添加到状态栏

    // --- 时间标签 ---
    m_timerLabel = new QLabel(QString::fromUtf8("时间: 0秒"));
    m_timerLabel->setMinimumWidth(120);
    m_timerLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_timerLabel);

    // --- 剩余标签 ---
    m_remainingLabel = new QLabel(QString::fromUtf8("剩余: 80"));
    m_remainingLabel->setMinimumWidth(100);
    m_remainingLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_remainingLabel);

    // --- 步数标签 ---
    m_movesLabel = new QLabel(QString::fromUtf8("步数: 0"));
    m_movesLabel->setMinimumWidth(100);
    m_movesLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_movesLabel);

    // ---- 第5步：创建游戏计时器 ----
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);  // 1000毫秒 = 1秒
    // 每隔1秒，m_gameTimer 发出 timeout() 信号 → onTimerTick() 被调用
    connect(m_gameTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    // ---- 第6步：连接 GameWidget 的信号 ----
    // 这是"控制器"模式的关键所在：
    //   GameWidget（画布）发生了一些事 → 发送信号
    //   → MainWindow（窗口）收到信号 → 更新状态栏

    // 分数变化 → 更新标签文字
    connect(m_gameWidget, &GameWidget::scoreChanged, this, [this](int s) {
        // QString::arg(s) = 把 %1 替换为 s 的值
        // 效果："分数: %1".arg(100) → "分数: 100"
        m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(s));
    });

    // 剩余方块数变化 → 更新标签
    connect(m_gameWidget, &GameWidget::tilesRemainingChanged, this, [this](int n) {
        m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(n));
    });

    // 步数变化 → 更新标签
    connect(m_gameWidget, &GameWidget::moveCountChanged, this, [this](int m) {
        m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(m));
    });

    // 通关 → 停止计时 + 弹窗
    connect(m_gameWidget, &GameWidget::gameWon, this, &MainWindow::onGameWon);

    // 无可用移动 → 状态栏显示提示
    connect(m_gameWidget, &GameWidget::noMovesLeft, this, &MainWindow::onNoMovesLeft);

    // ---- 第7步：开始新游戏 ----
    // 调用 GameWidget 的 startNewGame 生成初始棋盘
    m_gameWidget->startNewGame();
    // 启动计时器
    m_gameTimer->start();
    // 注意：计时器一旦 start()，就会每隔 interval 毫秒发出 timeout() 信号，
    // 直到调用 stop() 为止。
}

// ============================================================================
// 析构函数
// 由于所有子对象（GameWidget、QLabel、QTimer 等）的 parent 都设为了 this，
// Qt 的父子对象树会自动删除它们，所以这里不需要手动 delete。
// ============================================================================
MainWindow::~MainWindow()
{
}

// ============================================================================
// onNewGame() —— "新游戏"按钮点击处理
//
// 需要做的事情：
//   1. 让 GameWidget 重新生成棋盘
//   2. 计时归零
//   3. 如果计时器停了（上次通关后停了），重新启动
// ============================================================================
void MainWindow::onNewGame()
{
    m_gameWidget->startNewGame();       // 重置棋盘

    m_elapsedSeconds = 0;               // 计时归零
    m_timerLabel->setText(QString::fromUtf8("时间: 0秒"));

    // isActive() = 定时器是否在运行？
    // 如果上次通关后计时器停了，需要重新启动
    if (!m_gameTimer->isActive())
        m_gameTimer->start();
}

// ============================================================================
// onHint() —— "提示"按钮点击处理
//
// 直接委托给 GameWidget::showHint() 处理。
// 这就是"委托"模式：MainWindow 不做实际工作，把任务交给专门的类。
// ============================================================================
void MainWindow::onHint()
{
    m_gameWidget->showHint();
}

// ============================================================================
// onTimerTick() —— 计时器每秒触发的槽函数
//
// 更新状态栏的时间显示。
// 时间格式：
//   - 不到1分钟："时间: 42秒"
//   - 超过1分钟："时间: 1分23秒"
//
// 整数除法技巧：
//   min = m_elapsedSeconds / 60;  // 整除，得到分钟数
//   sec = m_elapsedSeconds % 60;  // 取余，得到秒数（0~59）
//   例如：83秒 → min=1, sec=23
// ============================================================================
void MainWindow::onTimerTick()
{
    m_elapsedSeconds++;  // 先加一秒

    int min = m_elapsedSeconds / 60;  // 分钟数（整除）
    int sec = m_elapsedSeconds % 60;  // 秒数（取余）

    if (min > 0)
        // 超过1分钟：显示"X分Y秒"
        m_timerLabel->setText(QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec));
    else
        // 不到1分钟：显示"Y秒"
        m_timerLabel->setText(QString::fromUtf8("时间: %1秒").arg(sec));
}

// ============================================================================
// onGameWon() —— 通关处理
//
// 被 GameWidget::gameWon 信号触发。
// 做的事情：停止计时，弹出"恭喜通关"对话框。
//
// QMessageBox::information 创建一个"信息"类型的弹窗：
//   参数1：父窗口（this = MainWindow）
//   参数2：标题
//   参数3：内容文字
//   效果：弹出一个带 OK 按钮的对话框
// ============================================================================
void MainWindow::onGameWon()
{
    m_gameTimer->stop();  // 停止计时器

    // 构造时间字符串（和 onTimerTick 一样的方式）
    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    QString timeStr;
    if (min > 0)
        timeStr = QString::fromUtf8("%1分%2秒").arg(min).arg(sec);
    else
        timeStr = QString::fromUtf8("%1秒").arg(sec);

    // 弹窗显示通关信息
    // \n = 换行符
    // QString 的链式 arg() 调用：
    //   arg(s1).arg(s2).arg(s3) 依次替换 %1, %2, %3
    QMessageBox::information(this,
        QString::fromUtf8("恭喜通关！"),           // 窗口标题
        QString::fromUtf8("太棒了，你成功消除了所有方块！\n\n"
                          "分数: %1\n"
                          "用时: %2\n"
                          "步数: %3")
            .arg(m_gameWidget->getScore())          // %1 = 分数
            .arg(timeStr)                            // %2 = 时间
            .arg(m_gameWidget->getMoves()));         // %3 = 步数
}

// ============================================================================
// onNoMovesLeft() —— 无可用移动处理
//
// 当 GameWidget 检测到棋盘需要自动重排时，在状态栏短暂显示提示。
// showMessage("文字", 显示毫秒数) 会在状态栏显示临时消息，
// 2秒（2000毫秒）后自动消失。
// ============================================================================
void MainWindow::onNoMovesLeft()
{
    ui.statusBar->showMessage(
        QString::fromUtf8("无可用移动，正在自动重排..."),
        2000);  // 显示2秒后自动消失
}
