// SettingsDialog.cpp — 设置对话框实现

#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFont>

// ---- 构造 ----

SettingsDialog::SettingsDialog(const GameSettings& current, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("游戏设置"));
    setMinimumWidth(380);
    setModal(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);

    // ---- 音量设置 ----
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

    // ---- 图标大小设置 ----
    QGroupBox* iconGroup = new QGroupBox(QString::fromUtf8("图标大小"), this);
    QHBoxLayout* iconLayout = new QHBoxLayout(iconGroup);

    m_iconSizeCombo = new QComboBox(this);
    m_iconSizeCombo->addItem(QString::fromUtf8("小"), 80);
    m_iconSizeCombo->addItem(QString::fromUtf8("正常"), 100);
    m_iconSizeCombo->addItem(QString::fromUtf8("大"), 120);

    int iconIdx = m_iconSizeCombo->findData(current.iconScale);
    if (iconIdx >= 0)
        m_iconSizeCombo->setCurrentIndex(iconIdx);

    iconLayout->addWidget(new QLabel(QString::fromUtf8("方块图标尺寸:"), this));
    iconLayout->addWidget(m_iconSizeCombo);
    iconLayout->addStretch();

    mainLayout->addWidget(iconGroup);

    // ---- 难度设置 ----
    QGroupBox* diffGroup = new QGroupBox(QString::fromUtf8("下一把难度"), this);
    QVBoxLayout* diffLayout = new QVBoxLayout(diffGroup);

    m_difficultyCombo = new QComboBox(this);
    m_difficultyCombo->addItem(
        QString::fromUtf8("简单 — 图案种类少，容易找到配对"),
        static_cast<int>(Difficulty::Easy));
    m_difficultyCombo->addItem(
        QString::fromUtf8("中等 — 标准难度，20种图案"),
        static_cast<int>(Difficulty::Normal));
    m_difficultyCombo->addItem(
        QString::fromUtf8("困难 — 图案种类多，考验眼力！"),
        static_cast<int>(Difficulty::Hard));

    int diffIdx = m_difficultyCombo->findData(
        static_cast<int>(current.difficulty));
    if (diffIdx >= 0)
        m_difficultyCombo->setCurrentIndex(diffIdx);

    m_difficultyHint = new QLabel(
        QString::fromUtf8("（难度在下一把新游戏生效，不影响当前进行中的游戏）"), this);
    m_difficultyHint->setStyleSheet("color: #888888; font-size: 11px;");

    diffLayout->addWidget(m_difficultyCombo);
    diffLayout->addWidget(m_difficultyHint);

    mainLayout->addWidget(diffGroup);

    // ---- 确定 / 取消 按钮 ----
    QDialogButtonBox* btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this);
    btnBox->button(QDialogButtonBox::Ok)->setText(
        QString::fromUtf8("确定"));
    btnBox->button(QDialogButtonBox::Cancel)->setText(
        QString::fromUtf8("取消"));

    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(btnBox);

    // 滑条变化 → 更新百分比标签
    connect(m_volumeSlider, &QSlider::valueChanged,
            this, &SettingsDialog::onVolumeChanged);
}

// ---- 事件处理 ----

void SettingsDialog::onVolumeChanged(int value)
{
    m_volumeLabel->setText(QString("%1%").arg(value));
}

GameSettings SettingsDialog::getSettings() const
{
    GameSettings s;
    s.volume = m_volumeSlider->value();
    s.iconScale = m_iconSizeCombo->currentData().toInt();
    s.difficulty = static_cast<Difficulty>(
        m_difficultyCombo->currentData().toInt());
    return s;
}
