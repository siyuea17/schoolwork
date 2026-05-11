#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_calculator.h"

class calculator : public QMainWindow
{
    Q_OBJECT

public:
    calculator(QWidget *parent = nullptr);
    ~calculator();

private:
    Ui::calculatorClass ui;
    bool calculate(double operand, const QString& pendingOperator);
    //终止运算，清除数据，报错
    void abortOperation();
    //连接信号和槽
    void connectSlots();

    //储存运算符
    QString pendingOperator;
    //储存运算结果
    double result;
    //标记是否等待一个操作数
    bool waitForOperand;

private slots:
    void on_clearBtn_clicked();
    void on_clearAllBtn_clicked();
    void on_equalBtn_clicked();
    void digitClicked();
    void on_signBtn_clicked();
    void operatorClicked();
    void on_pointBtn_clicked();

};

