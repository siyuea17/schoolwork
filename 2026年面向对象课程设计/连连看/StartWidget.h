#pragma once
// ============================================================================
// 文件：StartWidget.h
// 角色：游戏的"开始界面"——主菜单画面
//
// 提供三个功能按钮：
//   1. 继续上一次游戏 —— 从存档恢复中断的游戏
//   2. 新游戏 —— 用当前设置开始一局新的连连看
//   3. 设置 —— 打开设置对话框调整音量、图标大小、难度
//
// 设计思路：
//   这是一个独立的 QWidget，放在 QStackedWidget 的第0页。
//   按钮点击后发出信号，由 MainWindow 负责切换页面和启动游戏。
//   StartWidget 本身不管理游戏逻辑和设置逻辑，只管界面和发信号。
// ============================================================================

#include <QWidget>      // QWidget —— 所有界面控件的基类，StartWidget 继承它成为独立页面
#include <QPushButton>  // QPushButton —— 按钮控件，用于"继续/新游戏/设置"三个操作入口
#include <QLabel>        // QLabel —— 标签控件，用于显示标题和最高分
#include <QVBoxLayout>   // QVBoxLayout —— 垂直布局管理器，自动排列子控件的位置和大小

class StartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartWidget(QWidget* parent = nullptr);

    // 刷新"继续"按钮的可用状态（根据是否有存档）
    void refreshContinueButton();

signals:
    // ===== 三个主要操作信号 =====
    // 这些信号发给 MainWindow，由它负责具体执行
    void continueGame();   // 用户点击"继续上一次"
    void newGame();        // 用户点击"新游戏"
    void openSettings();   // 用户点击"设置"

private:
    QLabel* m_titleLabel;          // 标题大字："连连看"
    QLabel* m_highScoreLabel;      // 显示历史最高分
    QPushButton* m_continueBtn;    // "继续上一次"按钮
    QPushButton* m_newGameBtn;     // "新游戏"按钮
    QPushButton* m_settingsBtn;    // "设置"按钮
};
