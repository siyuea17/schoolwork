// ============================================================================
// 文件：MainWindow.cpp
// 角色："总控制器"——窗口怎么创建、页面怎么切换、音乐怎么放、存档怎么存，全部在这里
//
// 本文件是各个组件之间的"粘合剂"：
//   StartWidget 说"我要开始新游戏" → 这里创建 GameWidget 并切换页面
//   GameWidget 说"分数变了"      → 这里更新状态栏显示
//   GameWidget 说"通关了"        → 这里停计时 + 弹窗庆祝
//   玩家关窗口                   → 这里先弹出确认框，再保存进度
//
// 每一行代码都配有"如果……就……"风格的自然语言注释。
//
// 先理解 Qt 的几个关键概念：
//   connect(发信号者, 信号, 收信号者, 怎么处理) = "当发信号者发出信号时，自动调用后面的处理函数"
//   emit 信号名()     = "发出信号！"（像广播一样，所有连接的槽函数都会收到）
//   QStackedWidget    = "一摞纸"——好几页叠在一起，每次只能看到最上面那页
//   QMediaPlayer      = "播放器"——可以播放 .wav 文件（背景音乐、音效）
//   QSettings / JSON  = "记住东西"——把游戏进度写到文件里，下次能接着玩
// ============================================================================

#include "MainWindow.h"
#include "StartWidget.h"     // 自己写的开始界面（标题 + 三个按钮）
#include "GameWidget.h"      // 自己写的游戏画布（棋盘 + 绘制 + 点击）
#include "SettingsDialog.h"  // 自己写的设置弹窗（音量/图标大小/难度）
#include <QMessageBox>       // Qt 的弹窗——显示"恭喜通关"或"确定退出吗？"
#include <QStatusBar>        // Qt 的状态栏——窗口底部那一条，显示分数/时间等信息
#include <QToolBar>          // Qt 的工具栏——窗口顶部那一条，放"返回/新游戏/提示"按钮
#include <QDir>              // Qt 的目录操作——处理文件路径
#include <QFileInfo>         // Qt 的文件信息查询——检查文件是否存在、获取绝对路径
#include <QApplication>      // Qt 应用程序类——可以获得 exe 所在目录
#include <QCloseEvent>       // Qt 的关闭事件——玩家的 × 按钮被点击时触发

// ============================================================================
// 构造函数 —— 创建主窗口对象时自动调用
//
// 这个函数很长，但要做的其实就一件事：把窗口的各个部分"组装"起来。
// 流程（每一步都为下一步做铺垫）：
//   1. 加载 .ui 文件（从设计师拖出来的界面布局）
//   2. 加载用户上次保存的设置
//   3. 初始化音频（背景音乐 + 音效）
//   4. 创建 QStackedWidget（装两个页面的"壳"）
//   5. 把开始界面放到壳的第 0 页
//   6. 放一个空的占位页面到壳的第 1 页
//   7. 创建状态栏标签（分数/时间/剩余/步数）
//   8. 把所有信号槽连接好（按钮点击 → 对应的处理函数）
// ============================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)            // 首先调用父类 QMainWindow 的构造函数
    , m_gameWidget(nullptr)          // 游戏画布指针暂时为空（等玩家点"新游戏"才创建）
    , m_scoreLabel(nullptr)          // 分数标签指针初始为空（下面会 new 出来）
    , m_timerLabel(nullptr)          // 时间标签同理
    , m_remainingLabel(nullptr)      // 剩余标签同理
    , m_movesLabel(nullptr)          // 步数标签同理
    , m_comboLabel(nullptr)          // 连击标签同理
    , m_gameTimer(nullptr)           // 游戏计时器暂时为空
    , m_elapsedSeconds(0)            // 游戏计时从 0 秒开始
    , m_bgMusic(nullptr)             // 背景音乐播放器暂时为空
    , m_audioOutput(nullptr)         // 音频输出设备暂时为空
    , m_matchSound(nullptr)          // 消除音效暂时为空
    , m_winSound(nullptr)            // 胜利音效暂时为空
    , m_hintSound(nullptr)           // 提示音效暂时为空
    , m_isRestoring(false)           // 标记：不是从存档恢复的状态
{
    // ===== 第1步：加载 .ui 文件 =====
    // ui.setupUi(this) 是由 UIC 编译器从 MainWindow.ui 自动生成的
    // 执行后，.ui 里定义的所有东西（工具栏、状态栏、按钮）都"活了"
    ui.setupUi(this);

    // ===== 第2步：设置窗口标题 =====
    setWindowTitle(QString::fromUtf8("连连看"));   // 窗口左上角显示"连连看"

    // ===== 第3步：从 settings.ini 加载玩家上次的设置 =====
    m_settings.load();               // 读取音量、图标大小、难度、最高分

    // ===== 第4步：初始化音频系统 =====
    initMusic();                     // 找 bgm.wav → 循环播放背景音乐
    initSoundEffects();              // 找 match/win/hint.wav → 预备三种音效

    // ===== 第5步：创建 QStackedWidget（页面容器） =====
    m_stack = new QStackedWidget(this); // 这个容器可以装多个页面，每次只显示一个

    // ---- page 0：开始界面（标题"连连看" + "继续/新游戏/设置"三个按钮） ----
    m_startWidget = new StartWidget(this);   // 创建开始界面对象
    m_stack->addWidget(m_startWidget);       // 把开始界面加入页面容器，索引 0

    // ---- page 1：游戏画布（先放一个空占位，等玩家点"新游戏"时替换） ----
    m_gameWidget = nullptr;                               // 还没创建游戏画布
    m_stack->addWidget(new QWidget(this));                 // 放一个空的占位页面，索引 1

    // 把 QStackedWidget 设为窗口的"中央控件"（占据窗口主要的空间）
    setCentralWidget(m_stack);

    // ===== 第6步：设置初始显示 =====
    m_stack->setCurrentIndex(0);                           // 先显示开始界面（索引0）
    m_startWidget->refreshContinueButton();                // 根据有没有存档，决定"继续"按钮亮不亮

    // 主界面不需要工具栏和状态栏，先隐藏起来
    ui.mainToolBar->setVisible(false);                     // 隐藏工具栏
    ui.statusBar->setVisible(false);                       // 隐藏状态栏

    // ===== 第7步：创建状态栏标签（虽然先隐藏，但内容先准备好） =====
    createStatusBar();                   // 创建 5 个标签：分数、时间、剩余、步数、连击

    // ===== 第8步：连接信号槽（"如果发生A，就做B"） =====
    // ---- 开始界面的三个按钮 → 各自的处理函数 ----
    connect(m_startWidget, &StartWidget::continueGame,    // 如果"继续上一次"按钮被点了
            this, &MainWindow::onContinueGame);            // 就执行 onContinueGame 函数
    connect(m_startWidget, &StartWidget::newGame,          // 如果"新游戏"按钮被点了
            this, &MainWindow::onNewGame);                 // 就执行 onNewGame 函数
    connect(m_startWidget, &StartWidget::openSettings,     // 如果"设置"按钮被点了
            this, &MainWindow::onOpenSettings);            // 就执行 onOpenSettings 函数

    // ---- 工具栏按钮（进入游戏后才显示） → 各自的处理函数 ----
    connect(ui.homeAction, &QAction::triggered,            // 如果"返回"按钮被点了
            this, &MainWindow::onReturnToMenu);             // 就执行 onReturnToMenu
    connect(ui.newGameAction, &QAction::triggered,          // 如果"新游戏"按钮被点了
            this, &MainWindow::onNewGame);                 // 就重新开始
    connect(ui.hintAction, &QAction::triggered,             // 如果"提示"按钮被点了
            this, &MainWindow::onHint);                    // 就执行 onHint

    // ---- 游戏计时器（每秒触发一次） ----
    m_gameTimer = new QTimer(this);                        // 创建计时器对象
    m_gameTimer->setInterval(1000);                        // 设置间隔 1000 毫秒 = 1 秒
    connect(m_gameTimer, &QTimer::timeout,                  // 如果计时器说"1 秒到了！"
            this, &MainWindow::onTimerTick);               // 就执行 onTimerTick（时间+1）
}

// ============================================================================
// 析构函数 —— 窗口销毁时自动调用
// 为什么是空的？因为 Qt 的父子对象树机制会自动删除所有子对象，
// 不需要手动 delete。写了反而会出 bug（重复释放）。
// ============================================================================
MainWindow::~MainWindow()
{
}

// ============================================================================
// findAudioFile() —— 在多个常见位置搜索指定的音频文件
//
// 为什么需要搜索？因为程序可能从不同目录运行：
//   VS 调试时：exe 在 x64/Debug/ 下，音频在 连连看/ 下
//   直接运行：音频文件和 exe 放在一起
//   所以多找几个位置，找到一个就返回
//
// 返回：找到的文件的完整路径。找不到返回空字符串。
// ============================================================================
QString MainWindow::findAudioFile(const QString& filename) const
{
    QString appDir = QApplication::applicationDirPath();   // exe 所在的文件夹
    QStringList searchPaths = {                             // 要搜索的路径列表
        appDir + "/" + filename,                            // exe 同目录
        appDir + "/../../../../连连看/" + filename,         // 源码目录（不同深度尝试）
        appDir + "/../../../连连看/" + filename,
        appDir + "/../../连连看/" + filename,
        "./" + filename,                                    // 当前工作目录
    };
    for (const QString& p : searchPaths)                    // 对于列表中的每条路径
    {
        QString absPath = QFileInfo(p).absoluteFilePath();  // 把相对路径转成绝对路径
        if (QFileInfo::exists(absPath))                     // 如果这个文件确实存在
            return absPath;                                 // 就返回它（找到了！）
    }
    return QString();                                       // 所有路径都没找到，返回空串
}

// ============================================================================
// initMusic() —— 初始化背景音乐
// 找 bgm.wav → 创建播放器 → 设为无限循环 → 开始播放
// ============================================================================
void MainWindow::initMusic()
{
    QString musicPath = findAudioFile("bgm.wav");          // 搜索 bgm.wav

    if (musicPath.isEmpty())                               // 如果没找到文件
    {
        qDebug("bgm.wav not found, running without music"); // 在调试窗口输出提示
        return;                                            // 没有音乐也不影响游戏，直接返回
    }

    qDebug("Loading bgm: %s", qPrintable(musicPath));      // 调试输出：加载了哪个文件

    // ---- 创建音频输出设备（把数字音频数据送到声卡） ----
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(m_settings.volume / 100.0);   // 按设置中的比例调音量（0.0~1.0）

    // ---- 创建媒体播放器（读取音频文件 + 解码 + 输出） ----
    m_bgMusic = new QMediaPlayer(this);
    m_bgMusic->setAudioOutput(m_audioOutput);              // 指定输出设备（必须！否则没声音）
    m_bgMusic->setSource(QUrl::fromLocalFile(musicPath));  // 设置要播放的文件
    m_bgMusic->setLoops(QMediaPlayer::Infinite);            // 设为无限循环——播完从头再来

    m_bgMusic->play();                                      // 开始播放
}

// ============================================================================
// initSoundEffects() —— 初始化三个短音效播放器
//
// 为什么不共用一个播放器？
//   如果只用一个播放器，连续消除两对时，第一个音效还没播完就被第二个覆盖——
//   玩家只听到第二声，第一声被"掐断"了。
//   三个独立的播放器 → 消除音和提示音可以同时响，连续消除音不互相打断。
// ============================================================================
void MainWindow::initSoundEffects()
{
    // ---- 消除音效 ----
    QString matchPath = findAudioFile("match.wav");        // 找 match.wav
    if (!matchPath.isEmpty())                              // 如果找到了文件
    {
        m_matchSound = new QMediaPlayer(this);              // 就创建一个新的播放器
        m_matchSound->setAudioOutput(new QAudioOutput(this));// 给它配置一个输出设备
        m_matchSound->audioOutput()->setVolume(m_settings.volume / 100.0); // 设音量
        m_matchSound->setSource(QUrl::fromLocalFile(matchPath)); // 加载音频文件
    }

    // ---- 胜利音效 ----
    QString winPath = findAudioFile("win.wav");
    if (!winPath.isEmpty())
    {
        m_winSound = new QMediaPlayer(this);
        m_winSound->setAudioOutput(new QAudioOutput(this));
        m_winSound->audioOutput()->setVolume(m_settings.volume / 100.0);
        m_winSound->setSource(QUrl::fromLocalFile(winPath));
    }

    // ---- 提示音效 ----
    QString hintPath = findAudioFile("hint.wav");
    if (!hintPath.isEmpty())
    {
        m_hintSound = new QMediaPlayer(this);
        m_hintSound->setAudioOutput(new QAudioOutput(this));
        m_hintSound->audioOutput()->setVolume(m_settings.volume / 100.0);
        m_hintSound->setSource(QUrl::fromLocalFile(hintPath));
    }
}

// ============================================================================
// createStatusBar() —— 创建状态栏上的 5 个文字标签
//
// 状态栏是窗口底部的横条，这里放 5 个文本标签用于显示游戏信息。
// QLabel 的 setStyleSheet() 用 CSS 语法设置字体样式（和网页 CSS 一样）。
// ============================================================================
void MainWindow::createStatusBar()
{
    QStatusBar* sb = ui.statusBar;                         // 获取 .ui 文件中定义的状态栏

    m_scoreLabel = new QLabel(QString::fromUtf8("分数: 0")); // 创建标签，初始显示"分数: 0"
    m_scoreLabel->setMinimumWidth(120);                     // 设置最小宽度，防止文字被挤压
    m_scoreLabel->setStyleSheet("font-weight: bold; font-size: 13px; padding: 0 8px;"); // 加粗+字号+边距
    sb->addWidget(m_scoreLabel);                           // 把标签加到状态栏上

    m_timerLabel = new QLabel(QString::fromUtf8("时间: 0秒"));
    m_timerLabel->setMinimumWidth(120);
    m_timerLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_timerLabel);

    m_remainingLabel = new QLabel(QString::fromUtf8("剩余: 0"));
    m_remainingLabel->setMinimumWidth(100);
    m_remainingLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_remainingLabel);

    m_movesLabel = new QLabel(QString::fromUtf8("步数: 0"));
    m_movesLabel->setMinimumWidth(100);
    m_movesLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_movesLabel);

    // 连击标签：初始为空，游戏中有连击时才显示"连击: x3"
    m_comboLabel = new QLabel(QString::fromUtf8(""));
    m_comboLabel->setMinimumWidth(120);
    m_comboLabel->setStyleSheet("font-weight: bold; color: #FF8C00; font-size: 13px; padding: 0 8px;"); // 橙色加粗
    sb->addWidget(m_comboLabel);
}

// ============================================================================
// applySettings() —— 把当前设置值（音量、图标大小）实际应用到各组件
// ============================================================================
void MainWindow::applySettings()
{
    if (m_audioOutput)                                      // 如果背景音乐的音频输出存在
        m_audioOutput->setVolume(m_settings.volume / 100.0); // 就把音量设为用户设置的值

    if (m_gameWidget)                                       // 如果游戏画布已经创建
        m_gameWidget->setIconScale(m_settings.iconScale);    // 就把图标大小设为用户设置的值
}

// ============================================================================
// onContinueGame() —— "继续上一次"按钮的处理函数
//
// 流程：
//   1. 从 save.dat 读取存档 → 2. 销毁旧画布 → 3. 用存档参数重建画布
//   → 4. 恢复棋盘布局 → 5. 恢复分数/时间/连击 → 6. 切入游戏
// ============================================================================
void MainWindow::onContinueGame()
{
    SavedGameState state = SaveManager::load();             // 从 save.dat 加载存档
    if (!state.hasSaved) return;                           // 如果没有存档，就直接返回（什么都不做）

    m_isRestoring = true;                                   // 标记：正在恢复存档（某些步骤会有特殊处理）

    // ---- 清理旧的 GameWidget ----
    if (m_gameWidget)                                       // 如果之前已经有游戏画布
    {
        m_stack->removeWidget(m_gameWidget);                // 先从页面容器中移除
        delete m_gameWidget;                                // 然后销毁旧对象（释放内存）
        m_gameWidget = nullptr;                             // 指针置空
    }

    // ---- 用存档参数创建新的 GameWidget ----
    m_gameWidget = new GameWidget(state.rows, state.cols,   // 棋盘尺寸用存档里的
                                  state.tileTypes, state.copies, // 图案参数用存档里的
                                  m_settings.iconScale, this);   // 图标大小用当前设置

    // ---- 替换堆叠窗口的 game 页面 ----
    QWidget* oldPage = m_stack->widget(1);                  // 获取第1页（旧占位页）
    m_stack->removeWidget(oldPage);                         // 移除旧页面
    delete oldPage;                                         // 销毁旧页面
    m_stack->insertWidget(1, m_gameWidget);                 // 在同样的位置插入新画布

    // ---- 恢复游戏状态 ----
    m_gameWidget->deserializeBoard(state.gridData);         // 从存档数据恢复棋盘布局
    m_gameWidget->setBoardScore(state.score);               // 恢复分数
    m_gameWidget->setBoardMoves(state.moves);               // 恢复步数
    m_gameWidget->setBoardRemainingTiles(state.remainingTiles); // 恢复剩余方块数
    m_gameWidget->setBoardComboCount(state.comboCount);     // 恢复连击数

    m_elapsedSeconds = state.elapsedSeconds;                 // 恢复用时（直接覆盖当前计时值）

    // ---- 重新连接信号槽（因为旧的 GameWidget 已销毁，信号连接也断了） ----
    connectGameSignals();

    // ---- 更新状态栏显示 ----
    m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(state.score));
    m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(state.remainingTiles));
    m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(state.moves));

    int min = m_elapsedSeconds / 60;                        // 分钟 = 总秒数 ÷ 60
    int sec = m_elapsedSeconds % 60;                        // 秒数 = 总秒数取余 60
    if (min > 0)                                            // 如果超过了 1 分钟
        m_timerLabel->setText(QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec));
    else                                                    // 否则（不到 1 分钟）
        m_timerLabel->setText(QString::fromUtf8("时间: %1秒").arg(sec));

    if (state.comboCount > 0)                               // 如果有连击
        m_comboLabel->setText(QString::fromUtf8("连击: x%1").arg(state.comboCount + 1));
    else                                                    // 否则
        m_comboLabel->setText("");                          // 清空连击标签

    // ---- 启动计时 + 切入游戏 ----
    if (!m_gameTimer->isActive())                           // 如果计时器没在跑
        m_gameTimer->start();                               // 就启动它
    ui.mainToolBar->setVisible(true);                       // 显示工具栏
    ui.statusBar->setVisible(true);                         // 显示状态栏
    m_stack->setCurrentIndex(1);                            // 切换到第 1 页（游戏界面）
    m_gameWidget->computeLayout();                          // 计算像素布局
    m_gameWidget->update();                                 // 重绘画面

    m_isRestoring = false;                                  // 恢复完成，清除标记
}

// ============================================================================
// onNewGame() —— "新游戏"按钮的处理函数
// 和"继续"的区别：不恢复任何数据，创建全新的随机棋盘
// ============================================================================
void MainWindow::onNewGame()
{
    m_settings.load();                                      // 重新加载设置（玩家可能在设置里改了难度）

    // 根据当前难度获取棋盘参数（图案种类、每种几个、棋盘几行几列）
    DifficultyParams dp = paramsForDifficulty(m_settings.difficulty);

    // ---- 清理旧的 GameWidget ----
    if (m_gameWidget)
    {
        m_stack->removeWidget(m_gameWidget);
        delete m_gameWidget;
        m_gameWidget = nullptr;
    }

    // ---- 用难度参数创建全新的 GameWidget ----
    m_gameWidget = new GameWidget(dp.rows, dp.cols,
                                  dp.tileTypes, dp.copies,
                                  m_settings.iconScale, this);

    // ---- 替换堆叠窗口的 game 页面 ----
    QWidget* oldPage = m_stack->widget(1);
    m_stack->removeWidget(oldPage);
    delete oldPage;
    m_stack->insertWidget(1, m_gameWidget);

    // ---- 开始新游戏 ----
    connectGameSignals();                                   // 连接信号槽
    clearSavedGame();                                       // 删除旧存档（新游戏不需要旧存档）
    m_gameWidget->startNewGame();                           // 让 GameWidget 重置一切 + 生成新棋盘

    // ---- 重置计时显示 ----
    m_elapsedSeconds = 0;
    m_timerLabel->setText(QString::fromUtf8("时间: 0秒"));
    m_comboLabel->setText("");

    if (!m_gameTimer->isActive())                           // 确保计时器在跑
        m_gameTimer->start();

    // ---- 切入游戏 ----
    applySettings();                                        // 把设置应用到游戏画布
    ui.mainToolBar->setVisible(true);
    ui.statusBar->setVisible(true);
    m_stack->setCurrentIndex(1);
}

// ============================================================================
// connectGameSignals() —— 把 GameWidget 的"通知"连接到 MainWindow 的"响应"
//
// 每次创建新的 GameWidget 都要重新调用这个函数。
// 因为 connect 绑定的是具体的对象——旧的 GameWidget 销毁后连接自动断开。
//
// lambda 表达式 [this](参数) { ... } 解读：
//   [this] = "我要用 this（MainWindow 的成员变量）"
//   (参数) = "信号传过来的数据"
//   { ... } = "收到信号后具体做什么"
// ============================================================================
void MainWindow::connectGameSignals()
{
    if (!m_gameWidget) return;                              // 如果画布不存在，就不连（安全处理）

    // ---- "分数变了" → 更新分数标签 ----
    // QString::arg(s) = 把 %1 替换为 s 的值
    connect(m_gameWidget, &GameWidget::scoreChanged, this, [this](int s) {
        m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(s)); // 标签文字变成"分数: 100"
    });

    // ---- "剩余方块数变了" → 更新剩余标签 ----
    connect(m_gameWidget, &GameWidget::tilesRemainingChanged, this, [this](int n) {
        m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(n));
    });

    // ---- "步数变了" → 更新步数标签 + 播放消除音效 ----
    connect(m_gameWidget, &GameWidget::moveCountChanged, this, [this](int m) {
        m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(m)); // 更新步数显示
        if (m > 0 && m_matchSound)                           // 如果步数大于0（说明消除了）并且音效存在
        {
            m_matchSound->setPosition(0);                    // 把播放位置跳到开头（重放）
            m_matchSound->play();                            // 播放消除音效
        }
    });

    // ---- "通关了" → 执行通关处理 ----
    connect(m_gameWidget, &GameWidget::gameWon, this, &MainWindow::onGameWon);

    // ---- "没路了" → 状态栏显示提示 ----
    connect(m_gameWidget, &GameWidget::noMovesLeft, this, &MainWindow::onNoMovesLeft);

    // ---- "连击数变了" → 更新连击标签 ----
    connect(m_gameWidget, &GameWidget::comboCountChanged, this, &MainWindow::onComboChanged);
}

// ============================================================================
// onHint() —— 工具栏"提示"按钮被点击时的处理
// ============================================================================
void MainWindow::onHint()
{
    // 只有当前在游戏界面（第1页）时才执行提示
    if (m_gameWidget && m_stack->currentIndex() == 1)       // 如果画布存在 并且 当前在游戏页面
    {
        m_gameWidget->showHint();                            // 调用画布的提示功能

        if (m_hintSound)                                     // 如果提示音效存在
        {
            m_hintSound->setPosition(0);                     // 跳到开头
            m_hintSound->play();                             // 播放
        }
    }
}

// ============================================================================
// onTimerTick() —— 计时器每秒触发一次，更新时间显示
// ============================================================================
void MainWindow::onTimerTick()
{
    m_elapsedSeconds++;                                      // 秒数 +1

    int min = m_elapsedSeconds / 60;                         // 整除 60 得到分钟数
    int sec = m_elapsedSeconds % 60;                         // 取余 60 得到秒数

    if (min > 0)                                             // 如果超过 1 分钟
        m_timerLabel->setText(
            QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec)); // 显示"X分Y秒"
    else                                                     // 否则不到 1 分钟
        m_timerLabel->setText(
            QString::fromUtf8("时间: %1秒").arg(sec));       // 显示"Y秒"
}

// ============================================================================
// onComboChanged() —— 连击数变化时，更新状态栏上的连击标签
// ============================================================================
void MainWindow::onComboChanged(int comboCount)
{
    if (comboCount >= 1)                                     // 如果连击数 >= 1（有连击）
        m_comboLabel->setText(
            QString::fromUtf8("连击: x%1").arg(comboCount + 1)); // 显示"连击: x2"、"连击: x3"……
    else                                                     // 否则（没有连击）
        m_comboLabel->setText("");                           // 清空标签
}

// ============================================================================
// onGameWon() —— 通关时被调用（GameWidget 发出 gameWon 信号）
//
// 流程：停计时 → 删存档 → 播胜利音 → 检查新纪录 → 弹窗庆祝 → 返回主菜单
// ============================================================================
void MainWindow::onGameWon()
{
    m_gameTimer->stop();                                     // 停止游戏计时

    clearSavedGame();                                        // 删除存档（通关了不需要继续）

    if (m_winSound)                                          // 如果胜利音效存在
    {
        m_winSound->setPosition(0);                          // 跳到开头
        m_winSound->play();                                  // 播放！
    }

    // ---- 格式化用时 ----
    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    QString timeStr;
    if (min > 0)
        timeStr = QString::fromUtf8("%1分%2秒").arg(min).arg(sec);
    else
        timeStr = QString::fromUtf8("%1秒").arg(sec);

    // ---- 检查是否刷新了最高分 ----
    int score = m_gameWidget ? m_gameWidget->getScore() : 0;
    bool newHighScore = false;
    if (score > m_settings.highScore)                        // 如果这次得分比历史最高分还高
    {
        m_settings.highScore = score;                        // 就更新最高分
        m_settings.save();                                   // 保存到 settings.ini
        newHighScore = true;                                 // 标记"新纪录"
    }

    // ---- 构造弹窗消息 ----
    QString msg = QString::fromUtf8(
        "太棒了，你成功消除了所有方块！\n\n"
        "难度: %1\n"
        "分数: %2\n"
        "用时: %3\n"
        "步数: %4")
        .arg(difficultyName(m_settings.difficulty))          // %1 → 难度名称
        .arg(score)                                          // %2 → 最终得分
        .arg(timeStr)                                        // %3 → 用时
        .arg(m_gameWidget ? m_gameWidget->getMoves() : 0);   // %4 → 步数

    if (newHighScore)                                        // 如果是新纪录
        msg += QString::fromUtf8("\n\n🎉 新纪录！");         // 追加一句祝贺

    // ---- 弹出恭喜对话框 ----
    QMessageBox::information(this,                           // 父窗口是 MainWindow
        QString::fromUtf8("恭喜通关！"),                      // 窗口标题
        msg);                                                // 窗口内容

    // ---- 返回主菜单 ----
    onReturnToMenu();
}

// ============================================================================
// onNoMovesLeft() —— GameWidget 检测到死局时触发
// 在状态栏显示一条临时消息，2 秒后自动消失
// ============================================================================
void MainWindow::onNoMovesLeft()
{
    // QStatusBar::showMessage(文字, 持续时间ms) = 显示一条临时的状态信息
    ui.statusBar->showMessage(
        QString::fromUtf8("无可用移动，正在自动重排..."),
        2000);                                               // 2000 毫秒 = 2 秒后消失
}

// ============================================================================
// onOpenSettings() —— "设置"按钮被点击时，弹出设置对话框
//
// exec() 是"模态执行"——对话框显示期间主窗口被锁住，必须关闭对话框才能继续。
// 返回 Accepted → 用户点了"确定"；返回 Rejected → 用户点了"取消"。
// ============================================================================
void MainWindow::onOpenSettings()
{
    SettingsDialog dlg(m_settings, this);                    // 用当前设置值构造对话框

    if (dlg.exec() == QDialog::Accepted)                     // 如果用户点了"确定"
    {
        m_settings = dlg.getSettings();                      // 从对话框获取新的设置值
        m_settings.save();                                   // 写入 settings.ini 保存
        applySettings();                                     // 让新设置立即生效

        m_startWidget->refreshContinueButton();              // 最高分可能变了，刷新开始界面
    }
    // 如果点了取消 → 什么都不做，原设置保持不变
}

// ============================================================================
// onReturnToMenu() —— 从游戏界面切回主菜单
// 保存进度 → 停止提示 → 隐藏工具栏 → 切页面
// ============================================================================
void MainWindow::onReturnToMenu()
{
    m_gameTimer->stop();                                     // 停止游戏计时

    // 如果还有方块没消完（游戏进行中），保存进度以备"继续上一次"
    if (m_gameWidget && m_gameWidget->getRemainingTiles() > 0) // 如果画布存在 且 还有方块
    {
        saveGameState();                                     // 保存进度到 save.dat
    }

    if (m_gameWidget)                                        // 如果画布存在
        m_gameWidget->clearHintTimer();                      // 停止提示闪烁

    // 主菜单不需要工具栏和状态栏
    ui.mainToolBar->setVisible(false);
    ui.statusBar->setVisible(false);

    m_stack->setCurrentIndex(0);                             // 切到第 0 页（开始界面）
    m_startWidget->refreshContinueButton();                  // 更新"继续"按钮的状态
}

// ============================================================================
// saveGameState() —— 把当前游戏的所有状态打包成一个 SavedGameState 对象
// 然后通过 SaveManager 存入 save.dat（JSON 格式）
// ============================================================================
void MainWindow::saveGameState()
{
    if (!m_gameWidget) return;                               // 如果画布不存在，不保存

    SavedGameState state;
    state.hasSaved = true;                                   // 标记有存档
    state.rows = m_gameWidget->boardRows();                  // 棋盘行数
    state.cols = m_gameWidget->boardCols();                  // 棋盘列数
    state.tileTypes = m_gameWidget->boardTileTypes();        // 图案种类
    state.copies = m_gameWidget->boardCopiesPerType();       // 每类副本数
    state.score = m_gameWidget->getScore();                  // 分数
    state.moves = m_gameWidget->getMoves();                  // 步数
    state.remainingTiles = m_gameWidget->getRemainingTiles();// 剩余方块数
    state.comboCount = m_gameWidget->getComboCount();        // 当前连击数
    state.elapsedSeconds = m_elapsedSeconds;                  // 已用时间
    state.difficulty = m_settings.difficulty;                 // 难度
    state.gridData = m_gameWidget->serializeBoard();         // 棋盘完整布局（含边界）

    SaveManager::save(state);                                // 写入 save.dat
}

// ============================================================================
// clearSavedGame() —— 删除存档文件
// ============================================================================
void MainWindow::clearSavedGame()
{
    SaveManager::clear();                                    // 删除 save.dat
}

// ============================================================================
// closeEvent() —— 玩家点击窗口右上角 × 时触发
//
// 流程：
//   1. 弹出确认框："确定要退出吗？"
//   2. 玩家选 Yes → 保存进度（如果正在玩） → 正常关闭
//   3. 玩家选 No  → 调用 event->ignore() 取消关闭
// ============================================================================
void MainWindow::closeEvent(QCloseEvent* event)
{
    // ---- 弹出确认对话框 ----
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,                                                // 父窗口
        QString::fromUtf8("退出确认"),                        // 标题
        QString::fromUtf8("确定要退出连连看吗？\n\n"
                          "游戏进度会自动保存。"),          // 内容（\n=换行）
        QMessageBox::Yes | QMessageBox::No,                  // 显示"是"和"否"两个按钮
        QMessageBox::No);                                    // 默认选中"否"

    if (reply != QMessageBox::Yes)                           // 如果玩家没选"是"（选了否或关了弹出框）
    {
        event->ignore();                                     // 取消关闭事件——窗口继续开着
        return;
    }

    // ---- 玩家选了"是"：保存进度后正常关闭 ----
    if (m_gameWidget && m_stack->currentIndex() == 1          // 如果正在游戏页面
        && m_gameWidget->getRemainingTiles() > 0)            // 并且还有方块没消完
    {
        m_gameTimer->stop();                                  // 停止计时
        saveGameState();                                     // 保存进度
    }

    QMainWindow::closeEvent(event);                          // 调用父类的关闭逻辑——真正关闭窗口
}
