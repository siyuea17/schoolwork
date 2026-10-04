#pragma once

#include <memory>

#include <QMainWindow>

#include "SeqList.h"
#include "SinglyLinkedList.h"

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

// 题目1：线性表的应用 —— 图形界面（Qt Widgets）
//
// 界面上一共三块：
//   1) 插入区：输入若干整数，一次插进两个表
//   2) 操作区：按元素值删除 / 在指定元素值后插入 / 单链表逆序 / 排序 / 清空
//   3) 显示区：上面实时显示两个表的全部元素，下面一行行记录每次操作的结果
//
// 界面上的每个按钮都直接调用两个类的方法，没有额外的封装层：
//   插入      -> SeqList::Insert / SinglyLinkedList::Append
//   按值删除  -> Delete(x)        （删掉所有相同值）
//   按值插入  -> InsertAfter(y,x) （找不到 y 就不插）
//   逆序      -> SinglyLinkedList::Reverse（原地改指针）
//   排序      -> Sort()
//   清空      -> Clear()
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);

private slots:
	void OnInsert();
	void OnDelete();
	void OnInsertAfter();
	void OnReverse();
	void OnSort();
	void OnClear();
	void Refresh();

private:
	void BuildUi();
	// 把 val 插到两个表的表尾；链表沿 next_ 指针追加，不通过位序查找。
	void AppendBoth(int val);
	void Log(const QString& text);

	std::unique_ptr<SeqList<int>> seq_;
	std::unique_ptr<SinglyLinkedList<int>> list_;

	QLineEdit* insertEdit_;
	QLineEdit* deleteEdit_;
	QLineEdit* targetEdit_;
	QLineEdit* valueEdit_;

	QLabel* seqLabel_;
	QLabel* listLabel_;
	QPlainTextEdit* log_;
};
