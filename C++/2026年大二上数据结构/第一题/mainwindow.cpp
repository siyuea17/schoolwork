#include "mainwindow.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
	// 顺序表初始容量，装满后 Insert 会自动翻倍扩容
	const int kCapacity = 20;

	// 顺序表：用位序 1..Length() 依次取元素，拼成 "10 20 30"
	QString SeqText(SeqList<int>& s)
	{
		QStringList parts;
		for (int i = 1; i <= s.Length(); i++)
		{
			int v = 0;
			if (s.GetData(i, v))
			{
				parts << QString::number(v);
			}
		}
		return parts.isEmpty() ? QStringLiteral("（空）") : parts.join(QStringLiteral(" "));
	}

	// 单链表：沿 next_ 指针走一遍
	QString ListText(SinglyLinkedList<int>& l)
	{
		QStringList parts;
		for (LinkNode<int>* p = l.GetHead(); p != nullptr; p = p->next_)
		{
			parts << QString::number(p->data_);
		}
		return parts.isEmpty() ? QStringLiteral("（空）") : parts.join(QStringLiteral(" "));
	}
}

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, seq_(std::make_unique<SeqList<int>>(kCapacity))
	, list_(std::make_unique<SinglyLinkedList<int>>())
	, insertEdit_(nullptr)
	, deleteEdit_(nullptr)
	, targetEdit_(nullptr)
	, valueEdit_(nullptr)
	, seqLabel_(nullptr)
	, listLabel_(nullptr)
	, log_(nullptr)
{
	BuildUi();
	Refresh();
	Log(QStringLiteral("两个线性表已创建：顺序表初始容量 %1，单链表为空表。").arg(kCapacity));
}

void MainWindow::BuildUi()
{
	setWindowTitle(QStringLiteral("题目1：线性表的应用 —— 顺序表 / 单链表"));

	QWidget* central = new QWidget(this);
	QVBoxLayout* root = new QVBoxLayout(central);

	// ---------- 插入区 ----------
	QGroupBox* insertBox = new QGroupBox(QStringLiteral("插入元素"), central);
	QHBoxLayout* insertLayout = new QHBoxLayout(insertBox);
	insertLayout->addWidget(new QLabel(QStringLiteral("整数（空格分隔，可一次多个）："), insertBox));
	insertEdit_ = new QLineEdit(insertBox);
	insertEdit_->setPlaceholderText(QStringLiteral("例如：12 45 7"));
	insertLayout->addWidget(insertEdit_, 1);
	QPushButton* insertButton = new QPushButton(QStringLiteral("插入到两个表"), insertBox);
	insertLayout->addWidget(insertButton);
	root->addWidget(insertBox);

	// ---------- 操作区 ----------
	QGroupBox* opBox = new QGroupBox(QStringLiteral("按元素值操作"), central);
	QHBoxLayout* opLayout = new QHBoxLayout(opBox);

	opLayout->addWidget(new QLabel(QStringLiteral("删除值为"), opBox));
	deleteEdit_ = new QLineEdit(opBox);
	deleteEdit_->setMaximumWidth(70);
	opLayout->addWidget(deleteEdit_);
	opLayout->addWidget(new QLabel(QStringLiteral("的全部元素"), opBox));
	QPushButton* deleteButton = new QPushButton(QStringLiteral("删除"), opBox);
	opLayout->addWidget(deleteButton);

	opLayout->addSpacing(16);
	opLayout->addWidget(new QLabel(QStringLiteral("在值为"), opBox));
	targetEdit_ = new QLineEdit(opBox);
	targetEdit_->setMaximumWidth(70);
	opLayout->addWidget(targetEdit_);
	opLayout->addWidget(new QLabel(QStringLiteral("的元素后插入"), opBox));
	valueEdit_ = new QLineEdit(opBox);
	valueEdit_->setMaximumWidth(70);
	opLayout->addWidget(valueEdit_);
	QPushButton* afterButton = new QPushButton(QStringLiteral("插入"), opBox);
	opLayout->addWidget(afterButton);

	opLayout->addSpacing(16);
	QPushButton* reverseButton = new QPushButton(QStringLiteral("单链表逆序"), opBox);
	opLayout->addWidget(reverseButton);
	QPushButton* sortButton = new QPushButton(QStringLiteral("两个表排序"), opBox);
	opLayout->addWidget(sortButton);
	QPushButton* clearButton = new QPushButton(QStringLiteral("清空"), opBox);
	opLayout->addWidget(clearButton);
	opLayout->addStretch();
	root->addWidget(opBox);

	// ---------- 显示区 ----------
	QGroupBox* showBox = new QGroupBox(QStringLiteral("两个表的当前内容"), central);
	QVBoxLayout* showLayout = new QVBoxLayout(showBox);
	seqLabel_ = new QLabel(showBox);
	listLabel_ = new QLabel(showBox);
	seqLabel_->setWordWrap(true);
	listLabel_->setWordWrap(true);
	showLayout->addWidget(seqLabel_);
	showLayout->addWidget(listLabel_);
	root->addWidget(showBox);

	// ---------- 操作记录 ----------
	QGroupBox* logBox = new QGroupBox(QStringLiteral("操作记录"), central);
	QVBoxLayout* logLayout = new QVBoxLayout(logBox);
	log_ = new QPlainTextEdit(logBox);
	log_->setReadOnly(true);
	logLayout->addWidget(log_);
	root->addWidget(logBox, 1);

	setCentralWidget(central);

	// ---------- 按钮接线 ----------
	connect(insertButton, &QPushButton::clicked, this, &MainWindow::OnInsert);
	connect(deleteButton, &QPushButton::clicked, this, &MainWindow::OnDelete);
	connect(afterButton, &QPushButton::clicked, this, &MainWindow::OnInsertAfter);
	connect(reverseButton, &QPushButton::clicked, this, &MainWindow::OnReverse);
	connect(sortButton, &QPushButton::clicked, this, &MainWindow::OnSort);
	connect(clearButton, &QPushButton::clicked, this, &MainWindow::OnClear);

	// 输入框里按回车等于点对应的按钮
	connect(insertEdit_, &QLineEdit::returnPressed, this, &MainWindow::OnInsert);
	connect(deleteEdit_, &QLineEdit::returnPressed, this, &MainWindow::OnDelete);
	connect(valueEdit_, &QLineEdit::returnPressed, this, &MainWindow::OnInsertAfter);

	resize(760, 560);
}

void MainWindow::AppendBoth(int val)
{
	// 追加到表尾：顺序表用位序，单链表直接沿指针追加。
	seq_->Insert(seq_->Length() + 1, val);
	list_->Append(val);
}

void MainWindow::OnInsert()
{
	const QString text = insertEdit_->text().trimmed();
	if (text.isEmpty())
	{
		Log(QStringLiteral("请先输入要插入的整数。"));
		return;
	}

	const QStringList parts = text.split(QRegularExpression(QStringLiteral("[\\s,，]+")), Qt::SkipEmptyParts);
	int count = 0;
	QStringList skipped;
	for (int i = 0; i < parts.size(); i++)
	{
		bool ok = false;
		const int v = parts.at(i).toInt(&ok);
		if (!ok || v >= 100)
		{
			skipped << parts.at(i);
			continue;
		}
		AppendBoth(v);
		count++;
	}

	if (count > 0)
	{
		Log(QStringLiteral("插入 %1 个元素：%2").arg(count).arg(text));
	}
	if (!skipped.isEmpty())
	{
		Log(QStringLiteral("以下输入不是小于 100 的整数，已跳过：%1")
			.arg(skipped.join(QStringLiteral(" "))));
	}

	insertEdit_->clear();
	Refresh();
}

void MainWindow::OnDelete()
{
	bool ok = false;
	const int v = deleteEdit_->text().trimmed().toInt(&ok);
	if (!ok || v >= 100)
	{
		Log(QStringLiteral("请先在“删除值为”里填一个小于 100 的整数。"));
		return;
	}

	// Delete 会把两个表里所有值等于 v 的元素都删掉
	const int seqBefore = seq_->Length();
	const int listBefore = list_->Size();
	const bool seqOk = seq_->Delete(v);
	const bool listOk = list_->Delete(v);

	Log(QStringLiteral("删除值为 %1 的全部元素：顺序表 %2 -> %3，单链表 %4 -> %5。")
		.arg(v).arg(seqBefore).arg(seq_->Length()).arg(listBefore).arg(list_->Size()));
	if (!seqOk && !listOk)
	{
		Log(QStringLiteral("（两个表里都没有 %1）").arg(v));
	}

	deleteEdit_->clear();
	Refresh();
}

void MainWindow::OnInsertAfter()
{
	bool okTarget = false;
	bool okValue = false;
	const int target = targetEdit_->text().trimmed().toInt(&okTarget);
	const int value = valueEdit_->text().trimmed().toInt(&okValue);
	if (!okTarget || !okValue || target >= 100 || value >= 100)
	{
		Log(QStringLiteral("请把两个框都填上小于 100 的整数。"));
		return;
	}

	// 只认第一个匹配的元素；找不到就不插、返回 false
	const bool seqOk = seq_->InsertAfter(target, value);
	const bool listOk = list_->InsertAfter(target, value);

	if (seqOk || listOk)
	{
		Log(QStringLiteral("在第一个 %1 之后插入 %2：顺序表%3，单链表%4。")
			.arg(target).arg(value)
			.arg(seqOk ? QStringLiteral("成功") : QStringLiteral("未找到该元素"))
			.arg(listOk ? QStringLiteral("成功") : QStringLiteral("未找到该元素")));
	}
	else
	{
		Log(QStringLiteral("两个表里都没有值为 %1 的元素，未插入。").arg(target));
	}

	targetEdit_->clear();
	valueEdit_->clear();
	Refresh();
}

void MainWindow::OnReverse()
{
	// 原地反转：只改 next_ 指针，不借助任何辅助数组或链表
	const int count = list_->Size();
	list_->Reverse();
	Log(QStringLiteral("单链表已原地逆序（结点数 %1 不变，未使用辅助数组/链表）。").arg(count));
	Refresh();
}

void MainWindow::OnSort()
{
	seq_->Sort();
	list_->Sort();
	Log(QStringLiteral("两个表都已按元素值升序排列。"));
	Refresh();
}

void MainWindow::OnClear()
{
	seq_->Clear();
	list_->Clear();
	Log(QStringLiteral("两个表已清空。"));
	Refresh();
}

void MainWindow::Refresh()
{
	seqLabel_->setText(QStringLiteral("顺序表（%1 个元素）：%2")
		.arg(seq_->Length()).arg(SeqText(*seq_)));
	listLabel_->setText(QStringLiteral("单链表（%1 个结点）：%2")
		.arg(list_->Size()).arg(ListText(*list_)));
}

void MainWindow::Log(const QString& text)
{
	log_->appendPlainText(text);
}
