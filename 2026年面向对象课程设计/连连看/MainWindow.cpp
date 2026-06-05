// ============================================================================
// 文件：MainWindow.cpp
// 角色：MainWindow 类的实现——窗口创建、页面管理、信号连接、音乐控制
// ============================================================================

#include "MainWindow.h"
#include "StartWidget.h"
#include "GameWidget.h"
#include "SettingsDialog.h"
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QCloseEvent>

// ============================================================================
// 构造函数 —— 创建整个窗口的所有内容
//
// 新的启动流程：
//   1. 加载 UI  2. 加载设置  3. 初始化音乐
//   4. 创建 QStackedWidget（包含开始界面 + 游戏界面占位）
//   5. 创建状态栏  6. 连接工具栏按钮
//   7. 显示开始界面（而不是直接开始游戏！）
// ============================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_gameWidget(nullptr)
    , m_scoreLabel(nullptr)
    , m_timerLabel(nullptr)
    , m_remainingLabel(nullptr)
    , m_movesLabel(nullptr)
    , m_comboLabel(nullptr)
    , m_gameTimer(nullptr)
    , m_elapsedSeconds(0)
    , m_bgMusic(nullptr)
    , m_audioOutput(nullptr)
    , m_matchSound(nullptr)
    , m_winSound(nullptr)
    , m_hintSound(nullptr)
    , m_isRestoring(false)
{
    // ---- 加载 UI 文件 ----
    ui.setupUi(this);

    // ---- 设置窗口标题 ----
    setWindowTitle(QString::fromUtf8("连连看"));

    // ---- 加载设置 ----
    m_settings.load();

    // ---- 初始化背景音乐 ----
    initMusic();
    initSoundEffects();

    // ---- 创建堆叠窗口 ----
    m_stack = new QStackedWidget(this);

    // ---- 创建开始界面（第0页） ----
    m_startWidget = new StartWidget(this);
    m_stack->addWidget(m_startWidget);  // index 0

    // ---- 创建游戏画布占位（第1页，稍后新游戏/继续时替换） ----
    // 先放一个空 widget 占位，等 onNewGame/onContinueGame 时重建
    m_gameWidget = nullptr;
    m_stack->addWidget(new QWidget(this));  // index 1 临时占位

    setCentralWidget(m_stack);

    // ---- 显示开始界面（工具栏和状态栏先隐藏） ----
    m_stack->setCurrentIndex(0);
    m_startWidget->refreshContinueButton();
    ui.mainToolBar->setVisible(false);
    ui.statusBar->setVisible(false);

    // ---- 创建状态栏标签 ----
    createStatusBar();
    // ---- 连接开始界面信号 ----
    connect(m_startWidget, &StartWidget::continueGame,
            this, &MainWindow::onContinueGame);
    connect(m_startWidget, &StartWidget::newGame,
            this, &MainWindow::onNewGame);
    connect(m_startWidget, &StartWidget::openSettings,
            this, &MainWindow::onOpenSettings);

    // ---- 连接工具栏按钮 ----
    // "返回"按钮 → 切回主菜单
    connect(ui.homeAction, &QAction::triggered,
            this, &MainWindow::onReturnToMenu);
    connect(ui.newGameAction, &QAction::triggered,
            this, &MainWindow::onNewGame);
    connect(ui.hintAction, &QAction::triggered,
            this, &MainWindow::onHint);

    // ---- 创建游戏计时器 ----
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);
    connect(m_gameTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
}

MainWindow::~MainWindow()
{
}


// ============================================================================
// findAudioFile() —— 搜索音频文件路径
// 在几个常见位置查找：exe同级目录、源码目录、源码的上级目录
// ============================================================================
QString MainWindow::findAudioFile(const QString& filename) const
{
    QString appDir = QApplication::applicationDirPath();
    QStringList searchPaths = {
        appDir + "/" + filename,
        appDir + "/../../../../连连看/" + filename,
        appDir + "/../../../连连看/" + filename,
        appDir + "/../../连连看/" + filename,
        "./" + filename,
    };
    for (const QString& p : searchPaths)
    {
        QString absPath = QFileInfo(p).absoluteFilePath();
        if (QFileInfo::exists(absPath))
            return absPath;
    }
    return QString();
}

// ============================================================================
// initMusic() —— 初始化背景音乐
// ============================================================================
void MainWindow::initMusic()
{
    QString musicPath = findAudioFile("bgm.wav");

    if (musicPath.isEmpty())
    {
        qDebug("bgm.wav not found, running without music");
        return;
    }

    qDebug("Loading bgm: %s", qPrintable(musicPath));

    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(m_settings.volume / 100.0);

    m_bgMusic = new QMediaPlayer(this);
    m_bgMusic->setAudioOutput(m_audioOutput);
    m_bgMusic->setSource(QUrl::fromLocalFile(musicPath));
    m_bgMusic->setLoops(QMediaPlayer::Infinite);

    m_bgMusic->play();
}

// ============================================================================
// initSoundEffects() —— 初始化短音效播放器
//
// 消除、胜利、提示每个都用一个独立的 QMediaPlayer 实例。
// 好处是可以同时播放（比如连续消除时），不会一个覆盖掉另一个。
// ============================================================================
void MainWindow::initSoundEffects()
{
    // ---- 消除音效 ----
    QString matchPath = findAudioFile("match.wav");
    if (!matchPath.isEmpty())
    {
        m_matchSound = new QMediaPlayer(this);
        m_matchSound->setAudioOutput(new QAudioOutput(this));
        m_matchSound->audioOutput()->setVolume(m_settings.volume / 100.0);
        m_matchSound->setSource(QUrl::fromLocalFile(matchPath));
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
// createStatusBar() —— 创建状态栏标签
// ============================================================================
void MainWindow::createStatusBar()
{
    QStatusBar* sb = ui.statusBar;

    m_scoreLabel = new QLabel(QString::fromUtf8("分数: 0"));
    m_scoreLabel->setMinimumWidth(120);
    m_scoreLabel->setStyleSheet("font-weight: bold; font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_scoreLabel);

    m_timerLabel = new QLabel(QString::fromUtf8("时间: 0秒"));
    m_timerLabel->setMinimumWidth(120);
    m_timerLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_timerLabel);

    m_remainingLabel = new QLabel(QString::fromUtf8("剩余: 80"));
    m_remainingLabel->setMinimumWidth(100);
    m_remainingLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_remainingLabel);

    m_movesLabel = new QLabel(QString::fromUtf8("步数: 0"));
    m_movesLabel->setMinimumWidth(100);
    m_movesLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_movesLabel);

    // ===== 连击标签（本次新增） =====
    m_comboLabel = new QLabel(QString::fromUtf8(""));
    m_comboLabel->setMinimumWidth(120);
    m_comboLabel->setStyleSheet("font-weight: bold; color: #FF8C00; font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_comboLabel);
}

// ============================================================================
// applySettings() —— 把当前设置应用到各组件
//
// 在进入游戏时调用，确保音量、图标大小等设置生效。
// 难度参数在 onNewGame() 中通过创建新 GameBoard 时传入。
// ============================================================================
void MainWindow::applySettings()
{
    // 应用音量
    if (m_audioOutput)
        m_audioOutput->setVolume(m_settings.volume / 100.0);

    // 应用图标大小（如果 GameWidget 已创建）
    if (m_gameWidget)
        m_gameWidget->setIconScale(m_settings.iconScale);
}

// ============================================================================
// onContinueGame() —— "继续上一次"按钮处理
//
// 流程：
//   1. 从 QSettings 加载存档
//   2. 用存档中的棋盘参数创建 GameWidget
//   3. 恢复棋盘数据、分数、步数、连击、时间
//   4. 切换到游戏界面
// ============================================================================
void MainWindow::onContinueGame()
{
    SavedGameState state = SaveManager::load();
    if (!state.hasSaved) return;  // 没存档，什么都不做

    m_isRestoring = true;

    // 用存档中的参数创建 GameWidget
    DifficultyParams dp = paramsForDifficulty(state.difficulty);

    // 删除旧的游戏画布占位
    if (m_gameWidget)
    {
        m_stack->removeWidget(m_gameWidget);
        delete m_gameWidget;
        m_gameWidget = nullptr;
    }

    m_gameWidget = new GameWidget(state.rows, state.cols,
                                  state.tileTypes, dp.copies,
                                  m_settings.iconScale, this);

    // 替换堆叠窗口第1页
    QWidget* oldPage = m_stack->widget(1);
    m_stack->removeWidget(oldPage);
    delete oldPage;
    m_stack->insertWidget(1, m_gameWidget);

    // 恢复棋盘数据
    m_gameWidget->deserializeBoard(state.gridData);
    m_gameWidget->setBoardScore(state.score);
    m_gameWidget->setBoardMoves(state.moves);
    m_gameWidget->setBoardRemainingTiles(state.remainingTiles);
    m_gameWidget->setBoardComboCount(state.comboCount);

    // 恢复时间
    m_elapsedSeconds = state.elapsedSeconds;

    // 连接信号
    connectGameSignals();

    // 更新状态栏并发射初始值
    m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(state.score));
    m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(state.remainingTiles));
    m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(state.moves));

    // 同步时间显示
    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    if (min > 0)
        m_timerLabel->setText(QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec));
    else
        m_timerLabel->setText(QString::fromUtf8("时间: %1秒").arg(sec));

    // 更新连击显示
    if (state.comboCount > 0)
        m_comboLabel->setText(QString::fromUtf8("连击: x%1").arg(state.comboCount));
    else
        m_comboLabel->setText("");

    // 启动计时器
    if (!m_gameTimer->isActive())
        m_gameTimer->start();

    // 进入游戏 → 显示工具栏和状态栏
    ui.mainToolBar->setVisible(true);
    ui.statusBar->setVisible(true);

    // 切换页面
    m_stack->setCurrentIndex(1);
    m_gameWidget->computeLayout();
    m_gameWidget->update();

    m_isRestoring = false;
}

// ============================================================================
// onNewGame() —— "新游戏"按钮处理
//
// 流程：
//   1. 重新加载设置（可能用户在设置里改了难度）
//   2. 根据难度参数创建新 GameWidget
//   3. 清除存档
//   4. 切换到游戏界面
// ============================================================================
void MainWindow::onNewGame()
{
    // 重新加载设置
    m_settings.load();

    // 获取当前难度的棋盘参数
    DifficultyParams dp = paramsForDifficulty(m_settings.difficulty);

    // 删除旧的游戏画布
    if (m_gameWidget)
    {
        m_stack->removeWidget(m_gameWidget);
        delete m_gameWidget;
        m_gameWidget = nullptr;
    }

    // 创建新游戏画布
    m_gameWidget = new GameWidget(GameBoard::DEFAULT_ROWS,
                                  GameBoard::DEFAULT_COLS,
                                  dp.tileTypes,
                                  dp.copies,
                                  m_settings.iconScale, this);

    // 替换堆叠窗口第1页
    QWidget* oldPage = m_stack->widget(1);
    m_stack->removeWidget(oldPage);
    delete oldPage;
    m_stack->insertWidget(1, m_gameWidget);

    // 连接信号
    connectGameSignals();

    // 清除旧存档
    clearSavedGame();

    // 开始新游戏
    m_gameWidget->startNewGame();

    // 重置时间
    m_elapsedSeconds = 0;
    m_timerLabel->setText(QString::fromUtf8("时间: 0秒"));
    m_comboLabel->setText("");

    // 启动计时器
    if (!m_gameTimer->isActive())
        m_gameTimer->start();

    // 应用设置
    applySettings();

    // 进入游戏 → 显示工具栏和状态栏
    ui.mainToolBar->setVisible(true);
    ui.statusBar->setVisible(true);

    // 切换页面
    m_stack->setCurrentIndex(1);
}

// ============================================================================
// connectGameSignals() —— 连接 GameWidget 的所有信号
//
// 每次创建新的 GameWidget 时都需要重新连接，
// 因为旧的 GameWidget 被 delete 后信号槽自动断开。
//
// 这里同时连接消除/提示/胜利的信号到对应的音效播放。
// ============================================================================
void MainWindow::connectGameSignals()
{
    if (!m_gameWidget) return;

    connect(m_gameWidget, &GameWidget::scoreChanged, this, [this](int s) {
        m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(s));
    });

    connect(m_gameWidget, &GameWidget::tilesRemainingChanged, this, [this](int n) {
        m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(n));
    });

    connect(m_gameWidget, &GameWidget::moveCountChanged, this, [this](int m) {
        m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(m));

        // 每次消除（步数+1）时播放消除音效
        if (m > 0 && m_matchSound)
        {
            m_matchSound->setPosition(0);  // 从头播放
            m_matchSound->play();
        }
    });

    connect(m_gameWidget, &GameWidget::gameWon, this, &MainWindow::onGameWon);

    connect(m_gameWidget, &GameWidget::noMovesLeft, this, &MainWindow::onNoMovesLeft);

    connect(m_gameWidget, &GameWidget::comboCountChanged, this, &MainWindow::onComboChanged);
}

// ============================================================================
// onHint() —— "提示"按钮处理
// ============================================================================
void MainWindow::onHint()
{
    if (m_gameWidget && m_stack->currentIndex() == 1)
    {
        m_gameWidget->showHint();

        // 播放提示音效
        if (m_hintSound)
        {
            m_hintSound->setPosition(0);
            m_hintSound->play();
        }
    }
}

// ============================================================================
// onTimerTick() —— 计时器每秒触发
// ============================================================================
void MainWindow::onTimerTick()
{
    m_elapsedSeconds++;

    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;

    if (min > 0)
        m_timerLabel->setText(QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec));
    else
        m_timerLabel->setText(QString::fromUtf8("时间: %1秒").arg(sec));
}

// ============================================================================
// onComboChanged() —— 连击数变化
//
// 在状态栏显示"连击: x3"，如果没有连击就清空。
// 颜色在 CSS 中已设为橙色。
// ============================================================================
void MainWindow::onComboChanged(int comboCount)
{
    if (comboCount >= 1)
        m_comboLabel->setText(QString::fromUtf8("连击: x%1").arg(comboCount + 1));
    else
        m_comboLabel->setText("");
}

// ============================================================================
// onGameWon() —— 通关处理
// ============================================================================
void MainWindow::onGameWon()
{
    m_gameTimer->stop();

    // 清除存档（已通关）
    clearSavedGame();

    // 播放胜利音效
    if (m_winSound)
    {
        m_winSound->setPosition(0);
        m_winSound->play();
    }

    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    QString timeStr;
    if (min > 0)
        timeStr = QString::fromUtf8("%1分%2秒").arg(min).arg(sec);
    else
        timeStr = QString::fromUtf8("%1秒").arg(sec);

    // 检查并更新最高分
    int score = m_gameWidget ? m_gameWidget->getScore() : 0;
    bool newHighScore = false;
    if (score > m_settings.highScore)
    {
        m_settings.highScore = score;
        m_settings.save();
        newHighScore = true;
    }

    // 弹窗
    QString msg = QString::fromUtf8(
        "太棒了，你成功消除了所有方块！\n\n"
        "难度: %1\n"
        "分数: %2\n"
        "用时: %3\n"
        "步数: %4")
        .arg(difficultyName(m_settings.difficulty))
        .arg(score)
        .arg(timeStr)
        .arg(m_gameWidget ? m_gameWidget->getMoves() : 0);

    if (newHighScore)
        msg += QString::fromUtf8("\n\n🎉 新纪录！");

    QMessageBox::information(this,
        QString::fromUtf8("恭喜通关！"), msg);

    // 返回主菜单
    onReturnToMenu();
}

// ============================================================================
// onNoMovesLeft() —— 无可用移动处理
// ============================================================================
void MainWindow::onNoMovesLeft()
{
    ui.statusBar->showMessage(
        QString::fromUtf8("无可用移动，正在自动重排..."), 2000);
}

// ============================================================================
// onOpenSettings() —— "设置"按钮处理
// ============================================================================
void MainWindow::onOpenSettings()
{
    SettingsDialog dlg(m_settings, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        // 用户点了"确定"→ 保存设置
        m_settings = dlg.getSettings();
        m_settings.save();
        applySettings();

        // 刷新开始界面（主要为了更新最高分显示）
        m_startWidget->refreshContinueButton();
    }
}

// ============================================================================
// onReturnToMenu() —— 从游戏返回主菜单
//
// 保存当前进度（如果游戏还没通关），然后切回开始界面。
// ============================================================================
void MainWindow::onReturnToMenu()
{
    m_gameTimer->stop();

    // 如果游戏还在进行中（没通关），保存进度
    if (m_gameWidget && m_gameWidget->getRemainingTiles() > 0)
    {
        saveGameState();
    }

    // 停止提示
    if (m_gameWidget)
        m_gameWidget->clearHintTimer();

    // 返回主界面 → 隐藏工具栏和状态栏
    ui.mainToolBar->setVisible(false);
    ui.statusBar->setVisible(false);

    // 切回开始界面
    m_stack->setCurrentIndex(0);
    m_startWidget->refreshContinueButton();
}

// ============================================================================
// saveGameState() —— 保存当前游戏进度
// ============================================================================
void MainWindow::saveGameState()
{
    if (!m_gameWidget) return;

    SavedGameState state;
    state.hasSaved = true;
    state.rows = m_gameWidget->boardRows();
    state.cols = m_gameWidget->boardCols();
    state.tileTypes = m_gameWidget->boardTileTypes();
    state.copies = m_gameWidget->boardCopiesPerType();
    state.score = m_gameWidget->getScore();
    state.moves = m_gameWidget->getMoves();
    state.remainingTiles = m_gameWidget->getRemainingTiles();
    state.comboCount = m_gameWidget->getComboCount();
    state.elapsedSeconds = m_elapsedSeconds;
    state.difficulty = m_settings.difficulty;
    state.gridData = m_gameWidget->serializeBoard();

    SaveManager::save(state);
}

// ============================================================================
// clearSavedGame() —— 清除存档
// ============================================================================
void MainWindow::clearSavedGame()
{
    SaveManager::clear();
}

// ============================================================================
// closeEvent() —— 窗口关闭事件
//
// 重写 QMainWindow::closeEvent，在用户点击窗口的 × 按钮时：
//   1. 如果正在游戏中 → 自动保存进度
//   2. 然后正常关闭窗口
//
// 这样玩家关闭窗口后，下次打开程序，"继续上一次"按钮就是亮的！
// 存档保存在 QSettings 中（Windows 上存注册表），
// 不会因为重新编译而丢失——
// 注册表数据不依赖 .exe 文件，只要程序组织名和应用名不变就不会丢。
// ============================================================================
void MainWindow::closeEvent(QCloseEvent* event)
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        QString::fromUtf8("退出确认"),
        QString::fromUtf8("确定要退出连连看吗？\n\n游戏进度会自动保存。"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        event->ignore();  // 用户取消关闭
        return;
    }

    // 如果游戏正在进行中（还有方块没消完），自动保存
    if (m_gameWidget && m_stack->currentIndex() == 1
        && m_gameWidget->getRemainingTiles() > 0)
    {
        m_gameTimer->stop();
        saveGameState();
    }

    // 正常关闭
    QMainWindow::closeEvent(event);
}
