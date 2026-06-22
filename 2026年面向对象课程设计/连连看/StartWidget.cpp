// ============================================================================
// 文件：StartWidget.cpp
// 角色："开始界面"——玩家看到的第一个画面，有三个大按钮
//
// 本文件实现了主菜单的界面布局和按钮响应。
// 每一行代码都配有"如果……就……"风格的自然语言注释。
// ============================================================================

#include "StartWidget.h"
#include "GameSettings.h" // 自己写的设置/存档结构（SaveManager 可以检查有没有存档）
#include <QFont>          // QFont —— Qt 的字体类，用来设置文字大小和粗细
#include <QApplication>   // QApplication —— Qt 的应用程序类，本文件暂未直接使用

// ============================================================================
// 构造函数 —— 创建开始界面时自动调用
// 把标题、最高分、三个按钮从上到下排列好，设置好样式
// ============================================================================
StartWidget::StartWidget(QWidget* parent)
    : QWidget(parent)                        // 首先调用父类 QWidget 的构造函数
{
    // ===== 创建垂直布局（让控件从上到下排列） =====
    QVBoxLayout* layout = new QVBoxLayout(this); // QVBoxLayout = "垂直盒子"——控件一个个往下叠
    layout->setAlignment(Qt::AlignCenter);        // 所有控件整体居中对齐

    // ---- 上方弹性空白（把控件推到中间，上下留白比例 2:3） ----
    layout->addStretch(2);                       // 加 2 份弹性空白——窗口变大时空白也会变大

    // ===== 标题大字 "连 连 看" =====
    m_titleLabel = new QLabel(QString::fromUtf8("连 连 看")); // 创建标签控件显示标题
    QFont titleFont;                             // 创建字体对象
    titleFont.setPointSize(36);                  // 字号设为 36 磅（很大）
    titleFont.setBold(true);                     // 加粗
    m_titleLabel->setFont(titleFont);             // 把字体应用到标签上
    m_titleLabel->setStyleSheet("color: #FFD700; background: transparent;"); // 金色文字 + 透明背景
    m_titleLabel->setAlignment(Qt::AlignCenter); // 文字居中对齐
    layout->addWidget(m_titleLabel);             // 把标题标签加入布局

    // ---- 标题下方小间距 ----
    layout->addSpacing(10);                      // 加 10 像素的固定间距

    // ===== "历史最高分" 标签 =====
    m_highScoreLabel = new QLabel(QString::fromUtf8("历史最高分: 0")); // 初始显示 0
    QFont hsFont;
    hsFont.setPointSize(14);                     // 字号 14（比标题小）
    m_highScoreLabel->setFont(hsFont);
    m_highScoreLabel->setStyleSheet("color: #CCCCCC; background: transparent;"); // 浅灰色文字
    m_highScoreLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_highScoreLabel);

    // ---- 按钮区上方间距 ----
    layout->addSpacing(40);                      // 40 像素间距——标题和按钮之间的呼吸空间

    // ===== 统一的按钮样式（CSS） =====
    // 定义了按钮的字体、颜色、边框、圆角、悬停效果、禁用效果
    // 三个按钮共用同一套样式，保持视觉一致
    QString btnStyle =
        "QPushButton {"
        "  font-size: 18px;"                    // 字号 18
        "  font-weight: bold;"                  // 加粗
        "  padding: 14px 60px;"                // 上下14px内边距，左右60px
        "  color: #FFFFFF;"                     // 文字：白色
        "  background-color: #3D566E;"           // 背景：蓝灰色
        "  border: 2px solid #5D7E9E;"          // 边框：2px实线，浅蓝灰
        "  border-radius: 10px;"                // 圆角半径 10px
        "}"
        "QPushButton:hover {"                    // 鼠标悬停在按钮上时
        "  background-color: #4D6E8E;"           // 背景变亮
        "  border-color: #8DB8D8;"              // 边框变亮
        "}"
        "QPushButton:pressed {"                  // 鼠标按下时
        "  background-color: #2C4E5E;"           // 背景变暗（按下反馈）
        "}"
        "QPushButton:disabled {"                 // 按钮被禁用时（灰色不可点）
        "  color: #666666;"                     // 文字变灰
        "  background-color: #2A3440;"           // 背景变暗（告诉玩家"不可用"）
        "  border-color: #3A4450;"              // 边框变暗
        "}";

    // ===== "继续上一次"按钮 =====
    m_continueBtn = new QPushButton(QString::fromUtf8("继续上一次")); // 创建按钮
    m_continueBtn->setStyleSheet(btnStyle);        // 应用样式
    m_continueBtn->setCursor(Qt::PointingHandCursor); // 鼠标移到按钮上时变成"手指"形状
    layout->addWidget(m_continueBtn, 0, Qt::AlignCenter); // 加入布局（居中对齐）

    layout->addSpacing(16);                     // 按钮之间 16px 间距

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

    // ---- 下方弹性空白（让按钮区的上下空间均衡） ----
    layout->addStretch(3);                       // 加 3 份弹性空白（比上方多，按钮偏上）

    // ===== 连接按钮的"被点击"信号到对应的"发出信号" =====
    // 当按钮被点击 → emit 对应的信号 → MainWindow 收到信号后执行对应的处理
    connect(m_continueBtn, &QPushButton::clicked,  // 如果"继续上一次"按钮被点击
            this, &StartWidget::continueGame);       // 就发出 continueGame 信号
    connect(m_newGameBtn, &QPushButton::clicked,    // 如果"新游戏"按钮被点击
            this, &StartWidget::newGame);            // 就发出 newGame 信号
    connect(m_settingsBtn, &QPushButton::clicked,   // 如果"设置"按钮被点击
            this, &StartWidget::openSettings);       // 就发出 openSettings 信号

    // ===== 设置背景色 =====
    // autoFillBackground = true 必须设置，否则 setStyleSheet 的背景色不生效
    setAutoFillBackground(true);
    setStyleSheet("background-color: #2C3E50;");   // 深蓝灰背景（和游戏棋盘一致的色调）
}

// ============================================================================
// refreshContinueButton() —— 更新"继续"按钮的状态
//
// 如果有存档文件存在 → "继续上一次"按钮亮起（可以点）
// 如果没有存档文件 → 按钮灰色不可点，鼠标移上去提示"没有可用的存档"
// ============================================================================
void StartWidget::refreshContinueButton()
{
    bool hasSave = SaveManager::hasSavedGame();      // 检查 save.dat 是否存在

    m_continueBtn->setEnabled(hasSave);              // 如果有存档就启用按钮，没有就禁用

    if (hasSave)                                     // 如果有存档
        m_continueBtn->setToolTip(                   // 就设置提示文字为
            QString::fromUtf8("加载上一次未完成的游戏"));
    else                                             // 否则（没有存档）
        m_continueBtn->setToolTip(                   // 就设置提示文字为
            QString::fromUtf8("没有可用的存档"));

    // 同时刷新"历史最高分"的显示
    GameSettings settings;
    settings.load();                                 // 从 settings.ini 加载
    m_highScoreLabel->setText(
        QString::fromUtf8("历史最高分: %1").arg(settings.highScore));
}
