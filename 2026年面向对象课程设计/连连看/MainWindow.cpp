// MainWindow.cpp — 主窗口实现

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

// ---- 构造与析构 ----

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
    , m_isPaused(false)              // 标记：游戏未暂停
{
    ui.setupUi(this);
    setWindowTitle(QString::fromUtf8("连连看"));

    m_settings.load();
    initMusic();
    initSoundEffects();

    // QStackedWidget: page 0 = 开始界面, page 1 = 游戏画布
    m_stack = new QStackedWidget(this);

    m_startWidget = new StartWidget(this);
    m_stack->addWidget(m_startWidget);

    m_gameWidget = nullptr;
    m_stack->addWidget(new QWidget(this));

    setCentralWidget(m_stack);

    m_stack->setCurrentIndex(0);
    m_startWidget->refreshContinueButton();

    ui.mainToolBar->setVisible(false);
    ui.statusBar->setVisible(false);

    createStatusBar();

    // 开始界面按钮信号
    connect(m_startWidget, &StartWidget::continueGame,
            this, &MainWindow::onContinueGame);
    connect(m_startWidget, &StartWidget::newGame,
            this, &MainWindow::onNewGame);
    connect(m_startWidget, &StartWidget::openSettings,
            this, &MainWindow::onOpenSettings);

    // 工具栏按钮信号
    connect(ui.homeAction, &QAction::triggered,
            this, &MainWindow::onReturnToMenu);
    connect(ui.pauseAction, &QAction::triggered,            // 如果"暂停"按钮被点了
            this, &MainWindow::onPause);                    // 就执行 onPause
    connect(ui.newGameAction, &QAction::triggered,
            this, &MainWindow::onNewGame);
    connect(ui.hintAction, &QAction::triggered,
            this, &MainWindow::onHint);

    // 游戏计时器
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);
    connect(m_gameTimer, &QTimer::timeout,
            this, &MainWindow::onTimerTick);
}

MainWindow::~MainWindow()
{
}

// ---- 音频 ----

// 在多个常见位置搜索音频文件（适应不同运行目录）
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

// 三种音效各用独立播放器，避免连续消除时互相打断
void MainWindow::initSoundEffects()
{
    QString matchPath = findAudioFile("match.wav");
    if (!matchPath.isEmpty())
    {
        m_matchSound = new QMediaPlayer(this);
        m_matchSound->setAudioOutput(new QAudioOutput(this));
        m_matchSound->audioOutput()->setVolume(m_settings.volume / 100.0);
        m_matchSound->setSource(QUrl::fromLocalFile(matchPath));
    }

    QString winPath = findAudioFile("win.wav");
    if (!winPath.isEmpty())
    {
        m_winSound = new QMediaPlayer(this);
        m_winSound->setAudioOutput(new QAudioOutput(this));
        m_winSound->audioOutput()->setVolume(m_settings.volume / 100.0);
        m_winSound->setSource(QUrl::fromLocalFile(winPath));
    }

    QString hintPath = findAudioFile("hint.wav");
    if (!hintPath.isEmpty())
    {
        m_hintSound = new QMediaPlayer(this);
        m_hintSound->setAudioOutput(new QAudioOutput(this));
        m_hintSound->audioOutput()->setVolume(m_settings.volume / 100.0);
        m_hintSound->setSource(QUrl::fromLocalFile(hintPath));
    }
}

// ---- 状态栏 ----

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

    m_remainingLabel = new QLabel(QString::fromUtf8("剩余: 0"));
    m_remainingLabel->setMinimumWidth(100);
    m_remainingLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_remainingLabel);

    m_movesLabel = new QLabel(QString::fromUtf8("步数: 0"));
    m_movesLabel->setMinimumWidth(100);
    m_movesLabel->setStyleSheet("font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_movesLabel);

    m_comboLabel = new QLabel(QString::fromUtf8(""));
    m_comboLabel->setMinimumWidth(120);
    m_comboLabel->setStyleSheet("font-weight: bold; color: #FF8C00; font-size: 13px; padding: 0 8px;");
    sb->addWidget(m_comboLabel);
}

void MainWindow::applySettings()
{
    if (m_audioOutput)
        m_audioOutput->setVolume(m_settings.volume / 100.0);

    if (m_gameWidget)
        m_gameWidget->setIconScale(m_settings.iconScale);
}

// ---- 游戏流程：继续 / 新游戏 / 返回菜单 ----

void MainWindow::onContinueGame()
{
    SavedGameState state = SaveManager::load();
    if (!state.hasSaved) return;

    m_isRestoring = true;

    if (m_gameWidget)
    {
        m_stack->removeWidget(m_gameWidget);
        delete m_gameWidget;
        m_gameWidget = nullptr;
    }

    m_gameWidget = new GameWidget(state.rows, state.cols,
                                  state.tileTypes, state.copies,
                                  m_settings.iconScale, this);

    QWidget* oldPage = m_stack->widget(1);
    m_stack->removeWidget(oldPage);
    delete oldPage;
    m_stack->insertWidget(1, m_gameWidget);

    m_gameWidget->deserializeBoard(state.gridData);
    m_gameWidget->setBoardScore(state.score);
    m_gameWidget->setBoardMoves(state.moves);
    m_gameWidget->setBoardRemainingTiles(state.remainingTiles);
    m_gameWidget->setBoardComboCount(state.comboCount);

    m_elapsedSeconds = state.elapsedSeconds;

    connectGameSignals();

    m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(state.score));
    m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(state.remainingTiles));
    m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(state.moves));

    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    if (min > 0)
        m_timerLabel->setText(QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec));
    else
        m_timerLabel->setText(QString::fromUtf8("时间: %1秒").arg(sec));

    if (state.comboCount > 0)
        m_comboLabel->setText(QString::fromUtf8("连击: x%1").arg(state.comboCount + 1));
    else
        m_comboLabel->setText("");

    if (!m_gameTimer->isActive())
        m_gameTimer->start();
    ui.mainToolBar->setVisible(true);
    ui.statusBar->setVisible(true);
    m_stack->setCurrentIndex(1);
    m_gameWidget->computeLayout();
    m_gameWidget->update();

    m_isRestoring = false;
    m_isPaused = false;                                     // 清除暂停状态
    ui.pauseAction->setText(QString::fromUtf8("暂停"));     // 恢复暂停按钮文字为"暂停"
}

void MainWindow::onNewGame()
{
    // ---- 如果当前正在游戏界面，先暂停，然后弹出确认框 ----
    if (m_gameWidget && m_stack->currentIndex() == 1)
    {
        bool wasPlaying = !m_isPaused;                     // 记录弹出确认框前是否在游戏中

        // 如果当前没有暂停，就先暂停（这样操作被锁定、计时停止）
        if (wasPlaying)
        {
            pauseGame();                                   // 暂停游戏（计时停止、操作锁定）
        }

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            QString::fromUtf8("新游戏确认"),                // 窗口标题
            QString::fromUtf8("确定要开始新游戏吗？\n\n"
                              "当前游戏进度将会丢失。"),     // 内容
            QMessageBox::Yes | QMessageBox::No,             // 显示"是"和"否"两个按钮
            QMessageBox::No);                               // 默认选中"否"

        if (reply != QMessageBox::Yes)                      // 如果玩家没选"是"
        {
            // 玩家取消了——恢复之前的状态
            if (wasPlaying)
            {
                resumeGame();                               // 如果之前没暂停，就恢复
            }
            return;                                        // 什么都不做，结束
        }
        // 玩家选了"是"——继续执行下面的新游戏逻辑
    }

    m_settings.load();

    DifficultyParams dp = paramsForDifficulty(m_settings.difficulty);

    if (m_gameWidget)
    {
        m_stack->removeWidget(m_gameWidget);
        delete m_gameWidget;
        m_gameWidget = nullptr;
    }

    m_gameWidget = new GameWidget(dp.rows, dp.cols,
                                  dp.tileTypes, dp.copies,
                                  m_settings.iconScale, this);

    QWidget* oldPage = m_stack->widget(1);
    m_stack->removeWidget(oldPage);
    delete oldPage;
    m_stack->insertWidget(1, m_gameWidget);

    connectGameSignals();
    clearSavedGame();
    m_gameWidget->startNewGame();

    m_elapsedSeconds = 0;
    m_timerLabel->setText(QString::fromUtf8("时间: 0秒"));
    m_comboLabel->setText("");

    m_isPaused = false;                                     // 新游戏不暂停
    ui.pauseAction->setText(QString::fromUtf8("暂停"));     // 恢复按钮文字

    if (!m_gameTimer->isActive())
        m_gameTimer->start();

    applySettings();
    ui.mainToolBar->setVisible(true);
    ui.statusBar->setVisible(true);
    m_stack->setCurrentIndex(1);
}

// 每次创建新的 GameWidget 后重新连接信号槽
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
        if (m > 0 && m_matchSound)
        {
            m_matchSound->setPosition(0);
            m_matchSound->play();
        }
    });

    connect(m_gameWidget, &GameWidget::gameWon, this, &MainWindow::onGameWon);
    connect(m_gameWidget, &GameWidget::noMovesLeft, this, &MainWindow::onNoMovesLeft);

    connect(m_gameWidget, &GameWidget::comboCountChanged, this, &MainWindow::onComboChanged);
}

void MainWindow::onHint()
{
    if (m_gameWidget && m_stack->currentIndex() == 1)
    {
        m_gameWidget->showHint();

        if (m_hintSound)
        {
            m_hintSound->setPosition(0);
            m_hintSound->play();
        }
    }
}

// onPause() —— 工具栏"暂停"按钮被点击时的处理
// 点一下暂停（计时停止 + 操作锁定 + 按钮变"继续"），
// 再点一下继续（计时恢复 + 操作解锁 + 按钮变"暂停"）。
void MainWindow::onPause()
{
    // 只有当前在游戏界面时才处理暂停
    if (m_gameWidget && m_stack->currentIndex() == 1)       // 如果画布存在 并且 当前在游戏页面
    {
        if (m_isPaused)                                      // 如果当前已经是暂停状态
            resumeGame();                                    // 就恢复游戏（计时继续、操作解锁）
        else                                                 // 否则（当前是运行状态）
            pauseGame();                                     // 就暂停游戏（计时停止、操作锁定）
    }
}

// pauseGame() —— 暂停游戏
// 停止计时、通知 GameWidget 锁定操作、把按钮文字改为"继续"
void MainWindow::pauseGame()
{
    if (m_isPaused) return;                                  // 如果已经暂停了，就不再重复暂停

    m_gameTimer->stop();                                     // 停止游戏计时（onTimerTick不再被触发）
    m_isPaused = true;                                       // 标记：游戏已暂停
    ui.pauseAction->setText(QString::fromUtf8("继续"));     // 按钮文字改为"继续"——让玩家知道可以点此恢复

    if (m_gameWidget)                                        // 如果画布存在
        m_gameWidget->setPaused(true);                       // 通知画布：暂停了（画布会锁定鼠标操作 + 显示遮罩）
}

// resumeGame() —— 恢复（解除暂停）
// 启动计时、通知 GameWidget 解锁操作、把按钮文字改回"暂停"
void MainWindow::resumeGame()
{
    if (!m_isPaused) return;                                 // 如果没暂停，就不用恢复

    m_gameTimer->start();                                    // 重新启动游戏计时
    m_isPaused = false;                                      // 标记：游戏已恢复
    ui.pauseAction->setText(QString::fromUtf8("暂停"));     // 按钮文字恢复为"暂停"

    if (m_gameWidget)                                        // 如果画布存在
        m_gameWidget->setPaused(false);                      // 通知画布：恢复了（解除鼠标锁定 + 移除遮罩）
}

void MainWindow::onTimerTick()
{
    m_elapsedSeconds++;

    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;

    if (min > 0)
        m_timerLabel->setText(
            QString::fromUtf8("时间: %1分%2秒").arg(min).arg(sec));
    else
        m_timerLabel->setText(
            QString::fromUtf8("时间: %1秒").arg(sec));
}

void MainWindow::onComboChanged(int comboCount)
{
    if (comboCount >= 1)
        m_comboLabel->setText(
            QString::fromUtf8("连击: x%1").arg(comboCount + 1));
    else
        m_comboLabel->setText("");
}

void MainWindow::onGameWon()
{
    m_gameTimer->stop();
    clearSavedGame();

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

    int score = m_gameWidget ? m_gameWidget->getScore() : 0;
    bool newHighScore = false;
    if (score > m_settings.highScore)
    {
        m_settings.highScore = score;
        m_settings.save();
        newHighScore = true;
    }

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
        QString::fromUtf8("恭喜通关！"),
        msg);

    onReturnToMenu();
}

void MainWindow::onNoMovesLeft()
{
    ui.statusBar->showMessage(
        QString::fromUtf8("无可用移动，正在自动重排..."),
        2000);
}

void MainWindow::onOpenSettings()
{
    SettingsDialog dlg(m_settings, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        m_settings = dlg.getSettings();
        m_settings.save();
        applySettings();

        m_startWidget->refreshContinueButton();
    }
}

void MainWindow::onReturnToMenu()
{
    m_gameTimer->stop();

    if (m_gameWidget && m_gameWidget->getRemainingTiles() > 0)
    {
        saveGameState();
    }

    if (m_gameWidget)
        m_gameWidget->clearHintTimer();

    ui.mainToolBar->setVisible(false);
    ui.statusBar->setVisible(false);

    m_isPaused = false;                                      // 清除暂停状态
    ui.pauseAction->setText(QString::fromUtf8("暂停"));     // 重置暂停按钮文字

    m_stack->setCurrentIndex(0);
    m_startWidget->refreshContinueButton();
}

// ---- 存档 ----

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

void MainWindow::clearSavedGame()
{
    SaveManager::clear();
}

// ---- 关闭窗口 ----

void MainWindow::closeEvent(QCloseEvent* event)
{
    bool wasPlaying = false;                                 // 标记：弹出确认框之前游戏是否在运行
    if (m_gameWidget && m_stack->currentIndex() == 1         // 如果正在游戏页面
        && !m_isPaused)                                      // 且当前没有暂停
    {
        wasPlaying = true;                                   // 记录"刚才在玩"
        pauseGame();                                         // 先暂停游戏（让玩家看到确认框时游戏是冻结的）
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        QString::fromUtf8("退出确认"),
        QString::fromUtf8("确定要退出连连看吗？\n\n"
                          "游戏进度会自动保存。"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        // 玩家取消了——如果之前没暂停，就恢复游戏
        if (wasPlaying)                                      // 如果弹出确认框前在玩游戏
            resumeGame();                                    // 恢复游戏（计时继续、操作解锁）
        event->ignore();                                     // 取消关闭事件——窗口继续开着
        return;
    }

    if (m_gameWidget && m_stack->currentIndex() == 1
        && m_gameWidget->getRemainingTiles() > 0)
    {
        m_gameTimer->stop();
        saveGameState();
    }

    QMainWindow::closeEvent(event);
}
