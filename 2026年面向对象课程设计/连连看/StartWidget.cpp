// ============================================================================
// 文件：StartWidget.cpp
// 角色：StartWidget 的实现——画出主菜单，响应按钮点击
// ============================================================================

#include "StartWidget.h"
#include "GameSettings.h" // GameSettings —— 游戏设置结构体 + SaveManager 存档管理器
#include <QFont>          // QFont —— 字体描述，用于设置标题和分数文字的大小/粗细
#include <QApplication>   // QApplication —— 应用程序全局对象（此文件未直接使用，但保留以备用）

// ============================================================================
// 构造函数 —— 创建界面布局
//
// 布局结构（从上到下）：
//   [弹性空白]
//   "连连看" 大标题
//   "历史最高分: XXX"
//   [间距]
//   [继续上一次] 按钮
//   [新游戏]     按钮
//   [设置]       按钮
//   [弹性空白]
// ============================================================================
StartWidget::StartWidget(QWidget* parent)
    : QWidget(parent)
{
    // ===== 创建主布局（垂直盒子布局） =====
    // QVBoxLayout 会把子控件从上到下排列
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);  // 整体居中

    // ---- 上方弹性空白 ----
    layout->addStretch(2);

    // ===== 标题大字 =====
    m_titleLabel = new QLabel(QString::fromUtf8("连 连 看"));
    // 设置大号粗体字
    QFont titleFont;
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    // 设置颜色为金色
    m_titleLabel->setStyleSheet("color: #FFD700; background: transparent;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_titleLabel);

    // ---- 标题下方小间距 ----
    layout->addSpacing(10);

    // ===== 历史最高分 =====
    m_highScoreLabel = new QLabel(QString::fromUtf8("历史最高分: 0"));
    QFont hsFont;
    hsFont.setPointSize(14);
    m_highScoreLabel->setFont(hsFont);
    m_highScoreLabel->setStyleSheet("color: #CCCCCC; background: transparent;");
    m_highScoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_highScoreLabel);

    // ---- 按钮区上方间距 ----
    layout->addSpacing(40);

    // ===== 统一按钮样式 =====
    // 所有按钮用同一套 CSS，视觉风格一致，又不用重复写三遍
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

    // ===== "继续上一次"按钮 =====
    m_continueBtn = new QPushButton(QString::fromUtf8("继续上一次"));
    m_continueBtn->setStyleSheet(btnStyle);
    m_continueBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_continueBtn, 0, Qt::AlignCenter);

    layout->addSpacing(16);

    // ===== "新游戏"按钮 =====
    m_newGameBtn = new QPushButton(QString::fromUtf8("新游戏"));
    m_newGameBtn->setStyleSheet(btnStyle);
    m_newGameBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_newGameBtn, 0, Qt::AlignCenter);

    layout->addSpacing(16);

    // ===== "设置"按钮 =====
    m_settingsBtn = new QPushButton(QString::fromUtf8("设置"));
    m_settingsBtn->setStyleSheet(btnStyle);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_settingsBtn, 0, Qt::AlignCenter);

    // ---- 下方弹性空白 ----
    layout->addStretch(3);

    // ===== 信号连接 =====
    // 按钮点击 → 发射对应信号给 MainWindow
    connect(m_continueBtn, &QPushButton::clicked, this, &StartWidget::continueGame);
    connect(m_newGameBtn, &QPushButton::clicked, this, &StartWidget::newGame);
    connect(m_settingsBtn, &QPushButton::clicked, this, &StartWidget::openSettings);

    // ===== 设置背景色 =====
    // autoFillBackground 必须设为 true，否则 setStyleSheet 的背景色不生效
    setAutoFillBackground(true);
    setStyleSheet("background-color: #2C3E50;");
}

// ============================================================================
// refreshContinueButton() —— 根据是否有存档来启用/禁用"继续"按钮
//
// 在 MainWindow 构造时调用一次，每次从游戏返回菜单时也调用一次。
// 如果没有存档，按钮灰掉，告诉玩家"没有可以继续的进度"。
// ============================================================================
void StartWidget::refreshContinueButton()
{
    bool hasSave = SaveManager::hasSavedGame();
    m_continueBtn->setEnabled(hasSave);
    if (hasSave)
        m_continueBtn->setToolTip(QString::fromUtf8("加载上一次未完成的游戏"));
    else
        m_continueBtn->setToolTip(QString::fromUtf8("没有可用的存档"));

    // 同时刷新最高分显示
    GameSettings settings;
    settings.load();
    m_highScoreLabel->setText(
        QString::fromUtf8("历史最高分: %1").arg(settings.highScore));
}
