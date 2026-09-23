#include "calculator.h"
#include <QMessageBox>
#include <QPushButton>

calculator::calculator(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);            

    // 初始化状态
    ui.lineEdit->setText("0");
    result = 0.0;
    waitForOperand = true;

    connectSlots();              
}

calculator::~calculator() {}

void calculator::connectSlots()
{
    QPushButton* digitBtns[10] = {
        ui.digitBtn0, ui.digitBtn1, ui.digitBtn2, ui.digitBtn3,
        ui.digitBtn4, ui.digitBtn5, ui.digitBtn6, ui.digitBtn7,
        ui.digitBtn8, ui.digitBtn9
    };
    for (auto btn : digitBtns)
        connect(btn, &QPushButton::clicked, this, &calculator::digitClicked);

    QPushButton* operatorBtns[4] = {
        ui.addBtn, ui.subtractionBtn, ui.mulBtn, ui.divisionBtn
    };
    for (auto btn : operatorBtns)
        connect(btn, &QPushButton::clicked, this, &calculator::operatorClicked);
}

bool calculator::calculate(double operand, const QString& pendingOperator)
{
    if (pendingOperator == "+")
        result += operand;
    else if (pendingOperator == "-")
        result -= operand;
    else if (pendingOperator == "*")
        result *= operand;
    else if (pendingOperator == "/") {
        if (operand == 0.0) return false;
        result /= operand;
    }
    return true;
}

void calculator::abortOperation()
{
    result = 0.0;
    pendingOperator.clear();
    ui.lineEdit->setText("0");    // ✅ 改为 . 语法
    waitForOperand = true;
    QMessageBox::warning(this, "运算错误", "除数不能为零");
}

void calculator::digitClicked()
{
    QPushButton* digitBtn = qobject_cast<QPushButton*>(sender());
    if (!digitBtn) return;
    QString value = digitBtn->text();

    if (ui.lineEdit->text() == "0" && value == "0")
        return;

    if (waitForOperand) {
        ui.lineEdit->setText(value);
        waitForOperand = false;
    }
    else {
        ui.lineEdit->setText(ui.lineEdit->text() + value);
    }
}

void calculator::on_clearBtn_clicked()
{
    ui.lineEdit->setText("0");
    waitForOperand = true;
}

void calculator::on_clearAllBtn_clicked()
{
    ui.lineEdit->setText("0");
    waitForOperand = true;
    result = 0.0;
    pendingOperator.clear();
}

void calculator::on_equalBtn_clicked()
{
    double operand = ui.lineEdit->text().toDouble();
    if (pendingOperator.isEmpty())
        return;
    if (!calculate(operand, pendingOperator)) {
        abortOperation();
        return;
    }
    ui.lineEdit->setText(QString::number(result));
    pendingOperator.clear();
    result = 0.0;
    waitForOperand = true;
}

void calculator::on_signBtn_clicked()
{
    QString text = ui.lineEdit->text();
    double value = text.toDouble();
    if (value > 0)
        text.prepend('-');
    else if (value < 0)
        text.remove(0, 1);
    ui.lineEdit->setText(text);
}

void calculator::operatorClicked()
{
    QPushButton* clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;
    QString clickedOperator = clickedBtn->text();
    double operand = ui.lineEdit->text().toDouble();

    if (!pendingOperator.isEmpty()) {
        if (!calculate(operand, pendingOperator)) {
            abortOperation();
            return;
        }
        ui.lineEdit->setText(QString::number(result));
    }
    else {
        result = operand;
    }
    pendingOperator = clickedOperator;
    waitForOperand = true;
}

void calculator::on_pointBtn_clicked()
{
    if (waitForOperand)
        ui.lineEdit->setText("0");
    if (!ui.lineEdit->text().contains('.'))
        ui.lineEdit->setText(ui.lineEdit->text() + ".");
    waitForOperand = false;
}