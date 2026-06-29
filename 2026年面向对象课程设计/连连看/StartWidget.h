#pragma once
// StartWidget.h — 开始界面，提供继续游戏、新游戏、设置三个入口

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class StartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartWidget(QWidget* parent = nullptr);
    void refreshContinueButton();  // 根据是否有存档刷新"继续"按钮状态

signals:
    void continueGame();
    void newGame();
    void openSettings();

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_highScoreLabel = nullptr;
    QPushButton* m_continueBtn = nullptr;
    QPushButton* m_newGameBtn = nullptr;
    QPushButton* m_settingsBtn = nullptr;
};
