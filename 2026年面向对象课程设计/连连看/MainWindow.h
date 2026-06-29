#pragma once
// MainWindow.h — 主窗口，管理页面切换、背景音乐、状态栏、存档和暂停

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QStackedWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include "ui_MainWindow.h"
#include "GameSettings.h"

class GameWidget;
class StartWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // ---- 开始界面操作 ----
    void onContinueGame();
    void onNewGame();
    void onOpenSettings();
    void onReturnToMenu();

    // ---- 游戏内操作 ----
    void onHint();
    void onPause();              // 工具栏"暂停"按钮 → 切换暂停/继续，计时器停/开
    void onTimerTick();
    void onGameWon();
    void onNoMovesLeft();
    void onComboChanged(int comboCount);

private:
    // ---- 创建/初始化函数 ----
    void createStatusBar();
    void connectGameSignals();
    void initMusic();
    void initSoundEffects();
    QString findAudioFile(const QString& filename) const;
    void applySettings();

    // ---- 存档相关 ----
    void saveGameState();
    void clearSavedGame();
    void pauseGame();            // 暂停游戏：停计时、锁定画布、改按钮文字
    void resumeGame();           // 恢复游戏：启计时、解锁画布、改按钮文字

    Ui::MainWindowClass ui;

    // ---- 页面管理 ----
    QStackedWidget* m_stack = nullptr;
    StartWidget* m_startWidget = nullptr;
    GameWidget* m_gameWidget = nullptr;

    // ---- 状态栏标签 ----
    QLabel* m_scoreLabel = nullptr;
    QLabel* m_timerLabel = nullptr;
    QLabel* m_remainingLabel = nullptr;
    QLabel* m_movesLabel = nullptr;
    QLabel* m_comboLabel = nullptr;

    // ---- 游戏计时 ----
    QTimer* m_gameTimer = nullptr;
    int m_elapsedSeconds = 0;

    // ---- 背景音乐 ----
    QMediaPlayer* m_bgMusic = nullptr;
    QAudioOutput* m_audioOutput = nullptr;

    // ---- 短音效 ----
    QMediaPlayer* m_matchSound = nullptr;
    QMediaPlayer* m_winSound = nullptr;
    QMediaPlayer* m_hintSound = nullptr;

    // ---- 设置 ----
    GameSettings m_settings;

    // ---- 状态标记 ----
    bool m_isRestoring = false;  // 正在从存档恢复时跳过部分初始化
    bool m_isPaused = false;     // 游戏是否已暂停（计时停止、操作锁定）
};
