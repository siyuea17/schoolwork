// StartWidget.cpp — 开始界面实现

#include "StartWidget.h"
#include "GameSettings.h"
#include <QFont>
#include <QApplication>

// ---- 构造 ----

StartWidget::StartWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    layout->addStretch(2);

    // 标题
    m_titleLabel = new QLabel(QString::fromUtf8("连 连 看"));
    QFont titleFont;
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #FFD700; background: transparent;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_titleLabel);

    layout->addSpacing(10);

    // 历史最高分
    m_highScoreLabel = new QLabel(QString::fromUtf8("历史最高分: 0"));
    QFont hsFont;
    hsFont.setPointSize(14);
    m_highScoreLabel->setFont(hsFont);
    m_highScoreLabel->setStyleSheet("color: #CCCCCC; background: transparent;");
    m_highScoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_highScoreLabel);

    layout->addSpacing(40);

    // 统一按钮样式
    QString btnStyle =
        "QPushButton {"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "  padding: 14px 60px;"
        "  color: #FFFFFF;"
        "  background-color: #3D566E;"
        "  border: 2px solid #5D7E9E;"
        "  border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #4D6E8E;"
        "  border-color: #8DB8D8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #2C4E5E;"
        "}"
        "QPushButton:disabled {"
        "  color: #666666;"
        "  background-color: #2A3440;"
        "  border-color: #3A4450;"
        "}";

    // "继续上一次"按钮
    m_continueBtn = new QPushButton(QString::fromUtf8("继续上一次"));
    m_continueBtn->setStyleSheet(btnStyle);
    m_continueBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_continueBtn, 0, Qt::AlignCenter);

    layout->addSpacing(16);

    // "新游戏"按钮
    m_newGameBtn = new QPushButton(QString::fromUtf8("新游戏"));
    m_newGameBtn->setStyleSheet(btnStyle);
    m_newGameBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_newGameBtn, 0, Qt::AlignCenter);

    layout->addSpacing(16);

    // "设置"按钮
    m_settingsBtn = new QPushButton(QString::fromUtf8("设置"));
    m_settingsBtn->setStyleSheet(btnStyle);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_settingsBtn, 0, Qt::AlignCenter);

    layout->addStretch(3);

    // 按钮点击 → 发出信号
    connect(m_continueBtn, &QPushButton::clicked,
            this, &StartWidget::continueGame);
    connect(m_newGameBtn, &QPushButton::clicked,
            this, &StartWidget::newGame);
    connect(m_settingsBtn, &QPushButton::clicked,
            this, &StartWidget::openSettings);

    setAutoFillBackground(true);
    setStyleSheet("background-color: #2C3E50;");
}

// ---- 公共接口 ----

void StartWidget::refreshContinueButton()
{
    bool hasSave = SaveManager::hasSavedGame();

    m_continueBtn->setEnabled(hasSave);

    if (hasSave)
        m_continueBtn->setToolTip(
            QString::fromUtf8("加载上一次未完成的游戏"));
    else
        m_continueBtn->setToolTip(
            QString::fromUtf8("没有可用的存档"));

    GameSettings settings;
    settings.load();
    m_highScoreLabel->setText(
        QString::fromUtf8("历史最高分: %1").arg(settings.highScore));
}
