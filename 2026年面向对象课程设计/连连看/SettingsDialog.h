#pragma once
// SettingsDialog.h — 设置对话框，调整音量、图标大小、游戏难度

#include <QDialog>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "GameSettings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const GameSettings& current,
                            QWidget* parent = nullptr);
    GameSettings getSettings() const;

private slots:
    void onVolumeChanged(int value);

private:
    QSlider* m_volumeSlider = nullptr;
    QLabel* m_volumeLabel = nullptr;
    QComboBox* m_iconSizeCombo = nullptr;
    QComboBox* m_difficultyCombo = nullptr;
    QLabel* m_difficultyHint = nullptr;
};
