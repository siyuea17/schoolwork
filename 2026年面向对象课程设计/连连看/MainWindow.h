#pragma once
// ============================================================================
// 文件：MainWindow.h
// 角色：连连看的主窗口——管理开始界面、游戏界面、背景音乐和状态栏
//
// 本次更新：
//   1. 使用 QStackedWidget 管理两个页面：开始界面 / 游戏界面
//   2. 背景音乐循环播放（QMediaPlayer）
//   3. 游戏存档系统（支持"继续上一次"）
//   4. 设置系统（音量、图标大小、难度）
//   5. 返回主菜单功能
// ============================================================================

#include <QtWidgets/QMainWindow>          // QMainWindow —— Qt 标准主窗口，自带菜单栏/工具栏/状态栏
#include <QLabel>                          // QLabel —— 标签控件，用于状态栏显示分数/时间/步数等
#include <QTimer>                          // QTimer —— 定时器，用于游戏计时（每秒+1）
#include <QStackedWidget>                  // QStackedWidget —— 堆叠页面容器，管理开始界面/游戏界面的切换
#include <QtMultimedia/QMediaPlayer>       // QMediaPlayer —— 媒体播放器，用于循环播放背景音乐
#include <QtMultimedia/QAudioOutput>       // QAudioOutput —— 音频输出设备，控制音量和播放通道
#include "ui_MainWindow.h"                 // ui_MainWindow.h —— UIC 编译器从 MainWindow.ui 自动生成的 UI 代码
#include "GameSettings.h"                  // GameSettings —— 游戏设置和存档的数据结构

class GameWidget;   // 前向声明：告诉编译器 GameWidget 是一个类（避免循环include）
class StartWidget;  // 前向声明：告诉编译器 StartWidget 是一个类

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    // 窗口关闭事件——关闭前自动保存游戏进度
    void closeEvent(QCloseEvent* event) override;

private slots:
    // ===== 开始界面操作 =====
    void onContinueGame();       // "继续上一次"→ 加载存档进入游戏
    void onNewGame();            // "新游戏"→ 用当前设置创建新游戏
    void onOpenSettings();       // "设置"→ 弹出设置对话框
    void onReturnToMenu();       // 从游戏返回主菜单

    // ===== 游戏内操作 =====
    void onHint();               // 提示按钮
    void onTimerTick();          // 计时器每秒触发
    void onGameWon();            // 通关处理
    void onNoMovesLeft();        // 无可用移动提示
    void onComboChanged(int comboCount);  // 连击数变化

private:
    // ===== 创建函数（构造函数中调用） =====
    void createStatusBar();      // 创建状态栏标签
    void connectGameSignals();   // 连接 GameWidget 信号
    void initMusic();            // 初始化背景音乐
    void initSoundEffects();     // 初始化短音效（消除/胜利/提示）
    QString findAudioFile(const QString& filename) const;  // 查找音频文件路径
    void applySettings();        // 应用当前设置到各组件

    // ===== 以下为新增功能 =====
    void saveGameState();
    void clearSavedGame();

    // ===== 数据成员 =====
    Ui::MainWindowClass ui;

    // 页面管理
    QStackedWidget* m_stack;        // 堆叠窗口（切换开始界面/游戏界面）
    StartWidget* m_startWidget;     // 开始界面（第0页）
    GameWidget* m_gameWidget;       // 游戏画布（第1页）——每次新游戏重建

    // 状态栏标签
    QLabel* m_scoreLabel;
    QLabel* m_timerLabel;
    QLabel* m_remainingLabel;
    QLabel* m_movesLabel;
    QLabel* m_comboLabel;           // 连击数显示（本次新增）

    // 游戏计时
    QTimer* m_gameTimer;
    int m_elapsedSeconds;

    // 背景音乐
    QMediaPlayer* m_bgMusic;
    QAudioOutput* m_audioOutput;

    // 音效
    QMediaPlayer* m_matchSound;     // 消除音效
    QMediaPlayer* m_winSound;       // 胜利音效
    QMediaPlayer* m_hintSound;      // 提示音效

    // 设置
    GameSettings m_settings;

    // 是否从存档恢复（用于区分新游戏和继续游戏）
    bool m_isRestoring;
};
