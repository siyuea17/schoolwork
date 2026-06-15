// ============================================================================
// 文件：SettingsDialog.cpp
// 角色：设置对话框的实现——布局控件、处理交互
// ============================================================================

#include "SettingsDialog.h"
#include <QVBoxLayout>      // QVBoxLayout —— 垂直布局，子控件从上到下排列
#include <QHBoxLayout>      // QHBoxLayout —— 水平布局，子控件从左到右排列
#include <QGroupBox>        // QGroupBox —— 分组框，带标题的控件分组容器
#include <QDialogButtonBox> // QDialogButtonBox —— 标准对话框按钮组（确定/取消）
#include <QFont>             // QFont —— 字体描述，用于设置文字大小/粗细/样式

// ============================================================================
// 构造函数
// 参数 current：当前生效的设置值，用来初始化各个控件的默认状态
// ============================================================================
SettingsDialog::SettingsDialog(const GameSettings& current, QWidget* parent)
    : QDialog(parent)
{
    // ===== 窗口基本属性 =====
    setWindowTitle(QString::fromUtf8("游戏设置"));
    setMinimumWidth(380);
    // 模态对话框：必须关闭才能操作主窗口
    setModal(true);

    // ===== 主布局 =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);

    // ===================================================================
    // 音量设置组
    // ===================================================================
    QGroupBox* volumeGroup = new QGroupBox(QString::fromUtf8("音量"), this);
    QHBoxLayout* volLayout = new QHBoxLayout(volumeGroup);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(current.volume);
    m_volumeSlider->setMinimumWidth(200);
    m_volumeSlider->setTickPosition(QSlider::TicksBelow);
    m_volumeSlider->setTickInterval(10);

    m_volumeLabel = new QLabel(
        QString::fromUtf8("%1%").arg(current.volume), this);
    m_volumeLabel->setMinimumWidth(40);
    m_volumeLabel->setAlignment(Qt::AlignCenter);

    volLayout->addWidget(m_volumeSlider);
    volLayout->addWidget(m_volumeLabel);

    mainLayout->addWidget(volumeGroup);

    // ===================================================================
    // 图标大小设置组
    // ===================================================================
    QGroupBox* iconGroup = new QGroupBox(QString::fromUtf8("图标大小"), this);
    QHBoxLayout* iconLayout = new QHBoxLayout(iconGroup);

    m_iconSizeCombo = new QComboBox(this);
    m_iconSizeCombo->addItem(QString::fromUtf8("小"), 80);
    m_iconSizeCombo->addItem(QString::fromUtf8("正常"), 100);
    m_iconSizeCombo->addItem(QString::fromUtf8("大"), 120);

    // 找到当前值对应的下拉项
    int iconIdx = m_iconSizeCombo->findData(current.iconScale);
    if (iconIdx >= 0) m_iconSizeCombo->setCurrentIndex(iconIdx);

    iconLayout->addWidget(new QLabel(QString::fromUtf8("方块图标尺寸:"), this));
    iconLayout->addWidget(m_iconSizeCombo);
    iconLayout->addStretch();

    mainLayout->addWidget(iconGroup);

    // ===================================================================
    // 难度设置组
    // ===================================================================
    QGroupBox* diffGroup = new QGroupBox(QString::fromUtf8("下一把难度"), this);
    QVBoxLayout* diffLayout = new QVBoxLayout(diffGroup);

    m_difficultyCombo = new QComboBox(this);
    m_difficultyCombo->addItem(QString::fromUtf8("简单 — 6×8棋盘，48方块，8种图案"),
                                static_cast<int>(Difficulty::Easy));
    m_difficultyCombo->addItem(QString::fromUtf8("中等 — 10×10棋盘，100方块，20种图案"),
                                static_cast<int>(Difficulty::Normal));
    m_difficultyCombo->addItem(QString::fromUtf8("困难 — 12×14棋盘，168方块，28种图案"),
                                static_cast<int>(Difficulty::Hard));

    int diffIdx = m_difficultyCombo->findData(
        static_cast<int>(current.difficulty));
    if (diffIdx >= 0) m_difficultyCombo->setCurrentIndex(diffIdx);

    m_difficultyHint = new QLabel(
        QString::fromUtf8("（难度在下一把新游戏生效，不影响当前进行中的游戏）"), this);
    m_difficultyHint->setStyleSheet("color: #888888; font-size: 11px;");

    diffLayout->addWidget(m_difficultyCombo);
    diffLayout->addWidget(m_difficultyHint);

    mainLayout->addWidget(diffGroup);

    // ===================================================================
    // 确定/取消按钮
    // ===================================================================
    QDialogButtonBox* btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btnBox->button(QDialogButtonBox::Ok)->setText(
        QString::fromUtf8("确定"));
    btnBox->button(QDialogButtonBox::Cancel)->setText(
        QString::fromUtf8("取消"));

    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(btnBox);

    // ===== 滑块变化 → 更新标签显示 =====
    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onVolumeChanged);
}

// ============================================================================
// onVolumeChanged() —— 滑块值改变时同步更新百分比标签
// ============================================================================
void SettingsDialog::onVolumeChanged(int value)
{
    m_volumeLabel->setText(QString("%1%").arg(value));
}

// ============================================================================
// getSettings() —— 收集用户在对话框中设置的所有参数并返回
// ============================================================================
GameSettings SettingsDialog::getSettings() const
{
    GameSettings s;
    s.volume = m_volumeSlider->value();
    s.iconScale = m_iconSizeCombo->currentData().toInt();
    s.difficulty = static_cast<Difficulty>(
        m_difficultyCombo->currentData().toInt());
    return s;
}
