#pragma once

#include <iomanip>    // std::setw / std::left
#include <iostream>   // std::cout
#include <utility>    // std::swap

#include "LinearList.h"

template <class T>
struct LinkNode
{
	T data_;
	LinkNode<T>* next_;

	LinkNode(LinkNode<T>* ptr = nullptr);

	LinkNode(const T& item, LinkNode<T>* ptr = nullptr);
};

// 带头结点的单链表
//
// first_ 始终指向头结点，头结点不存放数据，它的 next_ 指向第一个真正的数据结点。
//   空表        ：first_->next_ == nullptr
//   遍历数据结点 ：从 first_->next_ 开始
// 有了头结点，插入/删除第一个位置就不用再特判“改表头”了，逻辑更统一。
template <class T>
class SinglyLinkedList : public LinearList<T>
{
public:
	SinglyLinkedList();

	SinglyLinkedList(const SinglyLinkedList<T>& other);

	~SinglyLinkedList();

	int Size() const override;

	int Search(const T& x) const override;

	int Locate(int i) const override;

	bool GetData(int i, T& x) const override;

	void SetData(int i, T& x) override;

	bool Insert(int i, const T& x) override;

	// 按指针追加到表尾，不通过位序查找。
	bool Append(const T& x);

	bool Remove(int i, T& x) override;

	bool IsEmpty() const override;

	bool IsFull() const override;

	void Sort() override;

	void Output() override;

	// 删除所有值等于 x 的结点；全程只用指针，不涉及下标
	bool Delete(const T& x) override;

	// 在第一个值等于 y 的结点之后插入值 x；找不到 y 时不插入并返回 false
	bool InsertAfter(const T& y, const T& x) override;

	// 原地逆序所有数据结点：只用三个指针改指向，不借助任何辅助数组或链表
	void Reverse();

	// 深拷贝赋值：不能直接用编译器生成的版本，那样会浅拷贝 first_ 指针，
	// 两个链表指向同一串结点，析构时会重复 delete
	SinglyLinkedList<T>& operator=(const SinglyLinkedList<T>& other);

	// 返回第一个数据结点（空表返回 nullptr）。头结点本身不对外暴露。
	LinkNode<T>* GetHead();
	const LinkNode<T>* GetHead() const;

	// 清空表：释放头结点之后的所有数据结点，头结点保留
	void Clear() override;

protected:
	// 取第 i 个数据结点（i 从 1 开始）；越界返回 nullptr
	LinkNode<T>* FindNode(int i);
	const LinkNode<T>* FindNode(int i) const;

	LinkNode<T>* first_;
	// 当前数据结点数量，Size() 直接返回它，避免每次遍历整条链表。
	int size_;
};

template <class T>
LinkNode<T>::LinkNode(LinkNode<T>* ptr)
{
	data_ = T();
	next_ = ptr;
}

template <class T>
LinkNode<T>::LinkNode(const T& item, LinkNode<T>* ptr)
{
	data_ = item;
	next_ = ptr;
}

template <class T>
SinglyLinkedList<T>::SinglyLinkedList()
{
	// 分配头结点，它的 data_ 不使用
	first_ = new LinkNode<T>();
	first_->next_ = nullptr;
	size_ = 0;
}

template <class T>
SinglyLinkedList<T>::SinglyLinkedList(const SinglyLinkedList<T>& other)
{
	first_ = new LinkNode<T>();
	first_->next_ = nullptr;
	size_ = 0;

	// 从 other 的第一个数据结点开始，逐个复制到本表尾部
	LinkNode<T>* tail = first_;
	for (const LinkNode<T>* p = other.first_->next_; p != nullptr; p = p->next_)
	{
		LinkNode<T>* node = new LinkNode<T>(p->data_);
		tail->next_ = node;
		tail = node;
		size_++;
	}
}

template <class T>
SinglyLinkedList<T>::~SinglyLinkedList()
{
	Clear();
	delete first_;   // 头结点本身也要释放
}

template <class T>
void SinglyLinkedList<T>::Clear()
{
	// 只删头结点之后的结点，头结点保留下来，表回到空表状态
	LinkNode<T>* p = first_->next_;
	while (p != nullptr)
	{
		LinkNode<T>* q = p->next_;
		delete p;
		p = q;
	}
	first_->next_ = nullptr;
	size_ = 0;
}

template <class T>
LinkNode<T>* SinglyLinkedList<T>::GetHead()
{
	return first_->next_;
}

template <class T>
const LinkNode<T>* SinglyLinkedList<T>::GetHead() const
{
	return first_->next_;
}

template <class T>
int SinglyLinkedList<T>::Size() const
{
	return size_;
}

template <class T>
int SinglyLinkedList<T>::Search(const T& x) const
{
	int i = 1;
	for (const LinkNode<T>* p = first_->next_; p != nullptr; p = p->next_)
	{
		if (p->data_ == x)
		{
			return i;
		}
		i++;
	}
	return 0;
}

template <class T>
LinkNode<T>* SinglyLinkedList<T>::FindNode(int i)
{
	if (i < 1)
	{
		return nullptr;
	}
	LinkNode<T>* p = first_->next_;
	int j = 1;
	while (p != nullptr && j < i)
	{
		p = p->next_;
		j++;
	}
	return p;
}

template <class T>
const LinkNode<T>* SinglyLinkedList<T>::FindNode(int i) const
{
	if (i < 1)
	{
		return nullptr;
	}
	const LinkNode<T>* p = first_->next_;
	int j = 1;
	while (p != nullptr && j < i)
	{
		p = p->next_;
		j++;
	}
	return p;
}

template <class T>
bool SinglyLinkedList<T>::IsEmpty() const
{
	return (first_->next_ == nullptr);
}

template <class T>
bool SinglyLinkedList<T>::IsFull() const
{
	return false;   // 链表只要内存够就能一直插，不存在“满”
}

template <class T>
int SinglyLinkedList<T>::Locate(int i) const
{
	return (FindNode(i) != nullptr) ? i : 0;
}

template <class T>
bool SinglyLinkedList<T>::GetData(int i, T& x) const
{
	const LinkNode<T>* p = FindNode(i);
	if (p == nullptr)
	{
		return false;
	}
	x = p->data_;
	return true;
}

template <class T>
void SinglyLinkedList<T>::SetData(int i, T& x)
{
	LinkNode<T>* p = FindNode(i);
	if (p != nullptr)
	{
		p->data_ = x;
	}
}

template <class T>
bool SinglyLinkedList<T>::Insert(int i, const T& x)
{
	// i 从 1 开始：插在第 i 个位置上；i == 表长+1 表示追加到表尾
	// 有了头结点，所有位置都是“在某个结点之后插入”，不用特判表头
	if (i < 1)
	{
		return false;
	}

	// 找“待插入位置的前一个结点”：i==1 时就是头结点
	LinkNode<T>* prev = (i == 1) ? first_ : FindNode(i - 1);
	if (prev == nullptr)
	{
		return false;
	}

	LinkNode<T>* node = new LinkNode<T>(x);
	node->next_ = prev->next_;
	prev->next_ = node;
	size_++;
	return true;
}

template <class T>
bool SinglyLinkedList<T>::Append(const T& x)
{
	LinkNode<T>* tail = first_;
	while (tail->next_ != nullptr)
	{
		tail = tail->next_;
	}

	LinkNode<T>* node = new LinkNode<T>(x);
	tail->next_ = node;
	size_++;
	return true;
}

template <class T>
bool SinglyLinkedList<T>::Remove(int i, T& x)
{
	if (i < 1)
	{
		return false;
	}

	// 待删结点的前驱：删第 1 个时前驱就是头结点（这正是带头结点的好处，
	// 不然就要单独写一段“改表头”的代码）
	LinkNode<T>* prev = (i == 1) ? first_ : FindNode(i - 1);
	if (prev == nullptr || prev->next_ == nullptr)
	{
		return false;
	}

	LinkNode<T>* doomed = prev->next_;
	x = doomed->data_;
	prev->next_ = doomed->next_;
	delete doomed;
	size_--;
	return true;
}

template <class T>
bool SinglyLinkedList<T>::Delete(const T& x)
{
	// prev 是最后一个保留下来的结点，cur 是当前检查的结点。
	// 有了头结点，prev 从 first_ 起步，删第一个数据结点也不用特判。
	LinkNode<T>* prev = first_;
	LinkNode<T>* cur = first_->next_;
	bool deleted = false;

	while (cur != nullptr)
	{
		if (cur->data_ == x)
		{
			prev->next_ = cur->next_;   // 摘链
			delete cur;
			cur = prev->next_;
			size_--;
			deleted = true;
		}
		else
		{
			prev = cur;
			cur = cur->next_;
		}
	}
	return deleted;
}

template <class T>
bool SinglyLinkedList<T>::InsertAfter(const T& y, const T& x)
{
	// 只找第一个值等于 y 的结点（题面：多个元素值仅考虑第一个）
	LinkNode<T>* p = first_->next_;
	while (p != nullptr && !(p->data_ == y))
	{
		p = p->next_;
	}
	if (p == nullptr)
	{
		return false;
	}

	LinkNode<T>* node = new LinkNode<T>(x);
	node->next_ = p->next_;
	p->next_ = node;
	size_++;
	return true;
}

template <class T>
void SinglyLinkedList<T>::Reverse()
{
	// 三指针原地反转（只反转头结点之后的数据结点，头结点位置不变）：
	// prev 是已经反转好的部分的新头，cur 是待处理结点。
	LinkNode<T>* prev = nullptr;
	LinkNode<T>* cur = first_->next_;
	while (cur != nullptr)
	{
		LinkNode<T>* next = cur->next_;   // 先存下一个
		cur->next_ = prev;                // 当前结点指回前面
		prev = cur;                       // prev 前移
		cur = next;                       // cur 前移
	}
	first_->next_ = prev;                 // 头结点接上反转后的链
}

template <class T>
void SinglyLinkedList<T>::Sort()
{
	// 与 SeqList 相同：只交换相邻元素的数据域，使用冒泡排序。
	// 排序过程中不新建数组或结点，也不改变结点之间的链接。
	for (LinkNode<T>* pass = first_->next_; pass != nullptr; pass = pass->next_)
	{
		for (LinkNode<T>* cur = first_->next_;
			cur != nullptr && cur->next_ != nullptr;
			cur = cur->next_)
		{
			if (cur->data_ > cur->next_->data_)
			{
				std::swap(cur->data_, cur->next_->data_);
			}
		}
	}
}

template <class T>
void SinglyLinkedList<T>::Output()
{
	for (LinkNode<T>* p = first_->next_; p != nullptr; p = p->next_)
	{
		std::cout << std::left << std::setw(8) << p->data_;
	}
	std::cout << std::endl;
}

template <class T>
SinglyLinkedList<T>& SinglyLinkedList<T>::operator=(const SinglyLinkedList<T>& other)
{
	// 深拷贝：先清掉自己的数据结点（头结点保留），再把对方的结点逐个复制过来
	if (this == &other)
	{
		return *this;
	}
	Clear();

	LinkNode<T>* tail = first_;
	for (LinkNode<T>* p = other.first_->next_; p != nullptr; p = p->next_)
	{
		LinkNode<T>* node = new LinkNode<T>(p->data_);
		tail->next_ = node;
		tail = node;
		size_++;
	}
	return *this;
}
