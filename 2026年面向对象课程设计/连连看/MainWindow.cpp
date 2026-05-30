#include "MainWindow.h"
#include "GameWidget.h"
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_gameWidget(nullptr)
    , m_scoreLabel(nullptr)
    , m_timerLabel(nullptr)
    , m_remainingLabel(nullptr)
    , m_movesLabel(nullptr)
    , m_gameTimer(nullptr)
    , m_elapsedSeconds(0)
{
    ui.setupUi(this);

    m_gameWidget = new GameWidget(this);
    setCentralWidget(m_gameWidget);

    connect(ui.newGameAction, &QAction::triggered,
            this, &MainWindow::onNewGame);
    connect(ui.hintAction, &QAction::triggered,
            this, &MainWindow::onHint);
    connect(ui.shuffleAction, &QAction::triggered,
            this, &MainWindow::onShuffle);

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

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000);
    connect(m_gameTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    connect(m_gameWidget, &GameWidget::scoreChanged, this, [this](int s) {
        m_scoreLabel->setText(QString::fromUtf8("分数: %1").arg(s));
    });
    connect(m_gameWidget, &GameWidget::tilesRemainingChanged, this, [this](int n) {
        m_remainingLabel->setText(QString::fromUtf8("剩余: %1").arg(n));
    });
    connect(m_gameWidget, &GameWidget::moveCountChanged, this, [this](int m) {
        m_movesLabel->setText(QString::fromUtf8("步数: %1").arg(m));
    });
    connect(m_gameWidget, &GameWidget::gameWon, this, &MainWindow::onGameWon);
    connect(m_gameWidget, &GameWidget::noMovesLeft, this, &MainWindow::onNoMovesLeft);

    m_gameWidget->startNewGame();
    m_gameTimer->start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::onNewGame()
{
    m_gameWidget->startNewGame();
    m_elapsedSeconds = 0;
    m_timerLabel->setText(QString::fromUtf8("时间: 0秒"));
    if (!m_gameTimer->isActive())
        m_gameTimer->start();
}

void MainWindow::onHint()
{
    m_gameWidget->showHint();
}

void MainWindow::onShuffle()
{
    m_gameWidget->shuffleBoard();
}

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

void MainWindow::onGameWon()
{
    m_gameTimer->stop();

    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    QString timeStr;
    if (min > 0)
        timeStr = QString::fromUtf8("%1分%2秒").arg(min).arg(sec);
    else
        timeStr = QString::fromUtf8("%1秒").arg(sec);

    QMessageBox::information(this,
        QString::fromUtf8("恭喜通关！"),
        QString::fromUtf8("太棒了，你成功消除了所有方块！\n\n"
                          "分数: %1\n"
                          "用时: %2\n"
                          "步数: %3")
            .arg(m_gameWidget->getScore())
            .arg(timeStr)
            .arg(m_gameWidget->getMoves()));
}

void MainWindow::onNoMovesLeft()
{
    QMessageBox::information(this,
        QString::fromUtf8("无可用移动"),
        QString::fromUtf8("当前没有可以消除的方块对。\n请点击【重排】按钮重新排列方块。"));
}
