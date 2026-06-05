#pragma once
// ============================================================================
// 文件：SettingsDialog.h
// 角色：设置对话框——让玩家调整音量、图标大小、游戏难度
//
// 模态对话框：打开后会挡住主窗口，必须点"确定"或"取消"才能回去。
// 这样设计是因为设置改动需要"一次性确认或放弃"。
// ============================================================================

#include <QDialog>       // QDialog —— Qt 对话框基类，提供模态/非模态弹窗功能
#include <QSlider>       // QSlider —— 滑动条控件，用于调节音量等连续数值
#include <QComboBox>     // QComboBox —— 下拉选择框，用于选择难度/图标大小等离散选项
#include <QLabel>         // QLabel —— 标签控件，用于显示文字说明
#include <QPushButton>    // QPushButton —— 按钮控件
#include "GameSettings.h" // GameSettings —— 游戏设置的"数据容器"结构体

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const GameSettings& current,
                            QWidget* parent = nullptr);

    // 获取用户在对话框中设置的参数
    GameSettings getSettings() const;

private slots:
    void onVolumeChanged(int value);

private:
    // ---- 控件 ----
    QSlider* m_volumeSlider;      // 音量滑块 0~100
    QLabel* m_volumeLabel;         // 显示当前音量百分比
    QComboBox* m_iconSizeCombo;   // 图标大小：小/正常/大
    QComboBox* m_difficultyCombo; // 难度：简单/中等/困难
    QLabel* m_difficultyHint;     // 提示文字："下一把游戏生效"
};
