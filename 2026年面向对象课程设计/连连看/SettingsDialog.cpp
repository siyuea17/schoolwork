// ============================================================================
// 文件：SettingsDialog.cpp
// 角色："设置弹窗"——让玩家调整音量、图标大小、游戏难度
//
// 每一个控件都有自然语言注释，方便初学者理解 Qt 对话框的创建流程。
// ============================================================================

#include "SettingsDialog.h"
#include <QVBoxLayout>      // QVBoxLayout —— 垂直布局：子控件从上到下排列
#include <QHBoxLayout>      // QHBoxLayout —— 水平布局：子控件从左到右排列
#include <QGroupBox>        // QGroupBox —— 分组框：带标题边框的那一组控件
#include <QDialogButtonBox> // QDialogButtonBox —— 标准按钮组：包含"确定"和"取消"按钮
#include <QFont>            // QFont —— 字体描述：设置文字的大小和粗细

// ============================================================================
// 构造函数 —— 创建对话框时自动调用
// current 参数：玩家当前的设置值，用来初始化对话框的默认状态
// ============================================================================
SettingsDialog::SettingsDialog(const GameSettings& current, QWidget* parent)
    : QDialog(parent)                        // 首先调用父类 QDialog 的构造函数
{
    // ===== 窗口基本属性 =====
    setWindowTitle(QString::fromUtf8("游戏设置")); // 窗口标题栏显示"游戏设置"
    setMinimumWidth(380);                    // 最小宽度 380 像素（防止太窄）
    setModal(true);                          // 设为"模态"——打开后主窗口被锁住

    // ===== 创建主布局（整个对话框都用这个垂直布局） =====
    QVBoxLayout* mainLayout = new QVBoxLayout(this); // 创建垂直布局
    mainLayout->setSpacing(16);              // 控件之间的间距设为 16 像素

    // ===================================================================
    // 第1个分组：音量设置
    // ===================================================================
    QGroupBox* volumeGroup = new QGroupBox(QString::fromUtf8("音量"), this); // 创建带"音量"标题的分组框
    QHBoxLayout* volLayout = new QHBoxLayout(volumeGroup); // 分组框内部用水平布局（滑条+标签并排）

    m_volumeSlider = new QSlider(Qt::Horizontal, this); // 创建水平滑条（左=0，右=100）
    m_volumeSlider->setRange(0, 100);        // 滑条范围：0 到 100
    m_volumeSlider->setValue(current.volume); // 把滑条初始值设为当前音量
    m_volumeSlider->setMinimumWidth(200);     // 滑条最小宽度 200px（够长才好拖）
    m_volumeSlider->setTickPosition(QSlider::TicksBelow); // 刻度线画在滑条下方
    m_volumeSlider->setTickInterval(10);      // 每 10 个单位画一条刻度线（0,10,20...100）

    m_volumeLabel = new QLabel(              // 创建标签显示当前值，如"70%"
        QString::fromUtf8("%1%").arg(current.volume), this);
    m_volumeLabel->setMinimumWidth(40);      // 最小宽度 40px
    m_volumeLabel->setAlignment(Qt::AlignCenter); // 文字居中

    volLayout->addWidget(m_volumeSlider);    // 把滑条加入水平布局
    volLayout->addWidget(m_volumeLabel);     // 把标签紧接着放在滑条右边

    mainLayout->addWidget(volumeGroup);      // 把整个音量分组加入主布局

    // ===================================================================
    // 第2个分组：图标大小设置
    // ===================================================================
    QGroupBox* iconGroup = new QGroupBox(QString::fromUtf8("图标大小"), this);
    QHBoxLayout* iconLayout = new QHBoxLayout(iconGroup);

    m_iconSizeCombo = new QComboBox(this);   // 下拉选择框——玩家三选一
    m_iconSizeCombo->addItem(QString::fromUtf8("小"), 80);    // 下拉项1："小"，数值=80
    m_iconSizeCombo->addItem(QString::fromUtf8("正常"), 100); // 下拉项2："正常"，数值=100
    m_iconSizeCombo->addItem(QString::fromUtf8("大"), 120);   // 下拉项3："大"，数值=120

    int iconIdx = m_iconSizeCombo->findData(current.iconScale); // 找出当前值对应的下拉项
    if (iconIdx >= 0)                            // 如果找到了对应的下拉项
        m_iconSizeCombo->setCurrentIndex(iconIdx); // 就选中它（保持和当前设置一致）

    iconLayout->addWidget(new QLabel(QString::fromUtf8("方块图标尺寸:"), this)); // 左侧说明文字
    iconLayout->addWidget(m_iconSizeCombo);      // 下拉框放在文字右边
    iconLayout->addStretch();                    // 弹性空白（把控件推到左边，右边留空）

    mainLayout->addWidget(iconGroup);

    // ===================================================================
    // 第3个分组：难度设置
    // ===================================================================
    QGroupBox* diffGroup = new QGroupBox(QString::fromUtf8("下一把难度"), this);
    QVBoxLayout* diffLayout = new QVBoxLayout(diffGroup); // 难度组内部用垂直布局（下拉+提示并排）

    m_difficultyCombo = new QComboBox(this);
    m_difficultyCombo->addItem(
        QString::fromUtf8("简单 — 图案种类少，容易找到配对"),
        static_cast<int>(Difficulty::Easy));     // 第一项："简单"，数值=Easy(0)
    m_difficultyCombo->addItem(
        QString::fromUtf8("中等 — 标准难度，20种图案"),
        static_cast<int>(Difficulty::Normal));   // 第二项："中等"，数值=Normal(1)
    m_difficultyCombo->addItem(
        QString::fromUtf8("困难 — 图案种类多，考验眼力！"),
        static_cast<int>(Difficulty::Hard));     // 第三项："困难"，数值=Hard(2)

    int diffIdx = m_difficultyCombo->findData(   // 找出当前难度对应的下拉项
        static_cast<int>(current.difficulty));
    if (diffIdx >= 0)                            // 如果找到了
        m_difficultyCombo->setCurrentIndex(diffIdx); // 就选中它

    m_difficultyHint = new QLabel(              // 小提示：告诉玩家难度什么时候生效
        QString::fromUtf8("（难度在下一把新游戏生效，不影响当前进行中的游戏）"), this);
    m_difficultyHint->setStyleSheet("color: #888888; font-size: 11px;"); // 灰色小字

    diffLayout->addWidget(m_difficultyCombo);    // 下拉框放上面
    diffLayout->addWidget(m_difficultyHint);     // 提示文字放下面

    mainLayout->addWidget(diffGroup);

    // ===================================================================
    // 底部：确定 / 取消 按钮
    // ===================================================================
    QDialogButtonBox* btnBox = new QDialogButtonBox( // 标准按钮组
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, // 包含"确定"和"取消"
        this);
    btnBox->button(QDialogButtonBox::Ok)->setText(     // 把"OK"的按钮文字改为
        QString::fromUtf8("确定"));                     // 中文"确定"
    btnBox->button(QDialogButtonBox::Cancel)->setText(  // 把"Cancel"的按钮文字改为
        QString::fromUtf8("取消"));                     // 中文"取消"

    // 连接"确定"按钮 → 对话框返回 Accepted（接受）
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    // 连接"取消"按钮 → 对话框返回 Rejected（拒绝）
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(btnBox);               // 按钮组放到主布局最后

    // ===== 连接滑条变化 → 更新百分比标签 =====
    connect(m_volumeSlider, &QSlider::valueChanged, // 如果滑条的值变了
            this, &SettingsDialog::onVolumeChanged); // 就调用 onVolumeChanged 更新标签
}

// ============================================================================
// onVolumeChanged() —— 滑条被拖动时，同步更新右边的百分比数字
// ============================================================================
void SettingsDialog::onVolumeChanged(int value)
{
    // 把标签文字更新为当前百分比，如 "70%"
    m_volumeLabel->setText(QString("%1%").arg(value));
}

// ============================================================================
// getSettings() —— 收集对话框里所有控件的值，打包成一个 GameSettings 返回
// MainWindow 调用这个函数来获得玩家修改后的设置
// ============================================================================
GameSettings SettingsDialog::getSettings() const
{
    GameSettings s;
    s.volume = m_volumeSlider->value();              // 从滑条读取音量值（0~100）
    s.iconScale = m_iconSizeCombo->currentData().toInt(); // 从下拉框读取缩放值（80/100/120）
    s.difficulty = static_cast<Difficulty>(          // 从下拉框读取难度枚举值
        m_difficultyCombo->currentData().toInt());
    return s;                                        // 返回打包好的设置
}
