#pragma once
// ============================================================================
// 文件：MainWindow.h
// 角色：连连看的主窗口——管理页面切换、背景音乐、状态栏和存档
//
// Qt 的 QMainWindow 是什么？
//   QMainWindow 是 Qt 提供的"标准应用程序窗口"类，自带以下布局区域：
//     ┌──────────────────────────────────┐
//     │  MenuBar（菜单栏）   ← QMenuBar   │
//     ├──────────────────────────────────┤
//     │  ToolBar（工具栏）   ← QToolBar   │ ← 返回、新游戏、提示按钮
//     ├──────────────────────────────────┤
//     │                                  │
//     │  CentralWidget（中央区域）        │ ← 这里放 QStackedWidget
//     │     ├─ StartWidget（开始界面）     │
//     │     └─ GameWidget（游戏画布）       │
//     │                                  │
//     ├──────────────────────────────────┤
//     │  StatusBar（状态栏）  ← QStatusBar │ ← 分数、时间、剩余、步数、连击
//     └──────────────────────────────────┘
//
//   这些区域 QMainWindow 自带，不需要从零写。只需要往里面填东西。
//
// 本次更新内容：
//   1. 使用 QStackedWidget 管理两个页面：开始界面 vs 游戏界面
//   2. 背景音乐循环播放 + 音效（消除/胜利/提示）
//   3. 游戏存档系统（关闭时自动保存，下次启动可"继续上一次"）
//   4. 设置系统（音量、图标大小、难度）
//   5. 返回主菜单 + 关闭确认弹窗
// ============================================================================

#include <QtWidgets/QMainWindow>          // QMainWindow —— Qt 标准主窗口类
                                          //   自带 MenuBar/ToolBar/StatusBar/CentralWidget 布局
#include <QLabel>                          // QLabel —— 标签控件，用于在状态栏显示文本
                                          //   如"分数: 100"、"时间: 1分23秒"等
#include <QTimer>                          // QTimer —— 定时器，每隔一段时间触发一次 timeout() 信号
                                          //   本文件用它每秒+1实现游戏计时
#include <QStackedWidget>                  // QStackedWidget —— 堆叠页面容器
                                          //   一个空间放多个页面，每次只显示一个（类似手机的分页）
                                          //   用 setCurrentIndex(n) 切换页面
                                          //   本文件用它管理：index0=开始界面, index1=游戏界面
#include <QtMultimedia/QMediaPlayer>       // QMediaPlayer —— Qt多媒体模块：媒体播放器
                                          //   支持播放 .wav/.mp3 等音频文件
                                          //   setLoops(Infinite) 可以无限循环（背景音乐）
                                          //   play()/stop()/setPosition(0) 控制播放
#include <QtMultimedia/QAudioOutput>       // QAudioOutput —— Qt多媒体模块：音频输出设备
                                          //   QMediaPlayer 不直接发声，而是把音频数据输出到
                                          //   QAudioOutput，由它负责真正播放声音
                                          //   setVolume(0.0~1.0) 控制音量
#include "ui_MainWindow.h"                 // ui_MainWindow.h —— UIC 编译器自动生成的代码
                                          //   UIC 读取 MainWindow.ui（XML）→ 生成 C++ 代码
                                          //   通过 ui.xxx 访问 .ui 文件中定义的控件
                                          //   例如 ui.newGameAction 就是"新游戏"按钮
#include "GameSettings.h"                  // GameSettings —— 游戏设置和存档的数据结构
                                          //   包含 GameSettings 结构体 + SaveManager 类

class GameWidget;   // 前向声明（Forward Declaration）
                    // 告诉编译器"GameWidget 是一个类"，不需要完整的头文件
                    // 好处：减少编译依赖，修改 GameWidget.h 不会导致 MainWindow 重新编译
class StartWidget;  // 前向声明同理

class MainWindow : public QMainWindow
{
    // Q_OBJECT 宏 —— 所有使用 signals/slots 的 Qt 类必须包含它
    // 编译时 Qt 的 MOC（Meta-Object Compiler，元对象编译器）会扫描这个宏，
    // 自动生成信号槽的底层实现代码。
    // 如果忘记写这个宏，connect() 等信号槽功能会在链接时报错。
    Q_OBJECT

public:
    // explicit = 禁止编译器进行隐式类型转换
    // 例如 explicit MainWindow(QWidget* p) 阻止了 MainWindow w = p; 这种意外写法
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    // closeEvent —— 窗口关闭事件（Qt 事件系统的一部分）
    // 当用户点击窗口右上角 × 按钮时，Qt 会调用这个函数
    // 重写（override）它可以在关闭前做额外处理——比如弹出确认框、保存进度
    // override 关键字：告诉编译器"我在重写父类的虚函数"，如果写错了签名编译器会报错
    void closeEvent(QCloseEvent* event) override;

private slots:
    // ===== 什么是 private slots？ =====
    // slots 关键字声明这些是"槽函数"——可以被信号触发的函数
    // private = 只有本类内部的 connect() 或 emit 能触发它们，外部不能直接调用
    // 从功能上讲它们和普通成员函数一样，只是多了一个"可以被信号连接"的超能力

    // ===== 开始界面操作 =====
    void onContinueGame();       // "继续上一次"→ 从 save.dat 加载存档 → 重建棋盘 → 切入游戏
    void onNewGame();            // "新游戏"→ 用当前难度设置创建新棋盘 → 清存档 → 切入游戏
    void onOpenSettings();       // "设置"→ 弹出 SettingsDialog 模态对话框
    void onReturnToMenu();       // 从游戏界面切回开始界面（自动保存进度）

    // ===== 游戏内操作 =====
    void onHint();               // 工具栏"提示"按钮 → 调 GameWidget::showHint() + 播放提示音效
    void onTimerTick();          // m_gameTimer 每 1000ms 触发 → elapsedSeconds++ → 更新时间显示
    void onGameWon();            // GameWidget 发出 gameWon() → 播放胜利音效 → 弹通关对话框
    void onNoMovesLeft();        // GameWidget 发出 noMovesLeft() → 状态栏显示短暂提示
    void onComboChanged(int comboCount);  // GameWidget 发出 comboCountChanged → 更新连击标签

private:
    // ===== 创建/初始化函数（在构造函数中调用） =====
    void createStatusBar();      // 创建状态栏的5个 QLabel（分数/时间/剩余/步数/连击）
    void connectGameSignals();   // 连接 GameWidget 的信号到 MainWindow 的槽/lambda
    void initMusic();            // 初始化背景音乐（bgm.wav，无限循环）
    void initSoundEffects();     // 初始化三个短音效（match.wav / win.wav / hint.wav）
    QString findAudioFile(const QString& filename) const;  // 在多个目录搜索音频文件
    void applySettings();        // 把 m_settings 的设置（音量/图标大小）应用到当前组件

    // ===== 存档相关 =====
    void saveGameState();        // 收集当前游戏所有状态 → SaveManager::save()
    void clearSavedGame();       // 删除 save.dat 存档文件

    // ======== 数据成员 ===========================================
    // 以下成员变量都在构造函数初始化列表中设为 nullptr/0，
    // 然后在构造函数体内逐步 new/创建。
    // Qt 父子对象树机制自动管理它们的生命周期。

    Ui::MainWindowClass ui;  // Ui::MainWindowClass —— UIC 生成的类
                             // 通过 ui.setupUi(this) 初始化
                             // 包含 .ui 文件定义的所有控件（工具栏按钮、状态栏等）

    // ---- 页面管理 ----
    QStackedWidget* m_stack;        // 堆叠页面容器，setCurrentIndex(0/1) 切换
                                    // index 0 = m_startWidget（开始界面）
                                    // index 1 = m_gameWidget（游戏画布）
    StartWidget* m_startWidget;     // 开始界面——"连连看"标题 + 三个按钮
    GameWidget* m_gameWidget;       // 游戏画布——每次新游戏或继续时重新 new

    // ---- 状态栏标签 ----
    // QLabel 是 Qt 的文本标签控件，用于显示不可编辑的静态文字
    // setStyleSheet() 使用 CSS 语法设置样式（字体、颜色、边距等）
    QLabel* m_scoreLabel;           // 显示"分数: 100"
    QLabel* m_timerLabel;           // 显示"时间: 1分23秒"
    QLabel* m_remainingLabel;       // 显示"剩余: 42"
    QLabel* m_movesLabel;           // 显示"步数: 15"
    QLabel* m_comboLabel;           // 显示"连击: x3"（橙色粗体，连击时显示）

    // ---- 游戏计时 ----
    QTimer* m_gameTimer;            // 每秒触发 onTimerTick() 的游戏计时器
    int m_elapsedSeconds;            // 游戏已进行的秒数（从 0 开始累加）

    // ---- 背景音乐 ----
    // QMediaPlayer + QAudioOutput 组合 = Qt 的音频播放方案
    // QMediaPlayer 负责"读文件 → 解码 → 输出数据"
    // QAudioOutput 负责"把数据送到声卡 → 真正发声"
    // 两者必须通过 setAudioOutput() 关联
    QMediaPlayer* m_bgMusic;        // 背景音乐播放器，setLoops(Infinite) 无限循环
    QAudioOutput* m_audioOutput;     // 音频输出设备，音量由它控制

    // ---- 短音效 ----
    // 三个独立的 QMediaPlayer，好处是可以同时播放
    // 例如连续消除时 matchSound 连续触发不会被自己打断
    // setPosition(0) 把播放位置移到开头，实现"重放"
    QMediaPlayer* m_matchSound;     // 消除音效（轻快的上扬音）
    QMediaPlayer* m_winSound;       // 胜利音效（上行琶音，末段渐弱）
    QMediaPlayer* m_hintSound;      // 提示音效（短促的叮咚声）

    // ---- 设置 ----
    GameSettings m_settings;         // 游戏设置（音量/图标大小/难度/最高分）
                                    // 在构造函数中 load()，修改后 save()

    // ---- 状态标记 ----
    bool m_isRestoring;              // true = 正在从存档恢复（跳过一些初始化步骤）
                                    // 用于区分"新游戏"和"继续上一次"的流程
};
