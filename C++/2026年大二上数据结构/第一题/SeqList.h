#pragma once

#include <cstddef>    // std::size_t
#include <cstdlib>    // exit
#include <iomanip>    // std::setw / std::left
#include <iostream>   // std::cerr / std::cout
#include <new>        // 定位 new
#include <utility>    // std::swap

#include "LinearList.h"

template <class T>
class SeqList : public LinearList<T>
{
protected:
	T* data_;
	int max_size_;
	int last_;

public:
	static const int kDefaultCapacity = 16;

	SeqList(int sz = kDefaultCapacity);

	SeqList(const SeqList<T>& other);

	~SeqList();

	int Size() const override;

	int Length() const;

	int Search(const T& x) const override;

	int Locate(int i) const override;

	bool GetData(int i, T& x) const override;

	void SetData(int i, T& x) override;

	bool Insert(int i, const T& x) override;

	bool Remove(int i, T& x) override;

	bool IsEmpty() const override;

	bool IsFull() const override;

	void Sort() override;

	void Output() override;

	// 删除所有值等于 x 的元素，删到至少一个返回 true
	bool Delete(const T& x) override;

	// 在第一个值等于 y 的元素之后插入 x；不存在 y 则不插入并返回 false
	bool InsertAfter(const T& y, const T& x) override;

	// 清空表：析构所有有效元素，数组空间保留给下次插入
	void Clear() override;

	// 深拷贝赋值：不能直接用编译器生成的版本，那样只会拷 data_ 指针
	SeqList<T>& operator=(const SeqList<T>& other);

protected:
	// 只在有效范围内（0 .. last_）有元素被构造过，
	// 所以释放内存前必须先把它们逐个析构，见 ~SeqList 和 Clear。
	void DestroyAll();

	// 分配一块能放下 new_size 个 T 的原始内存，不做任何构造
	static T* Allocate(int new_size);

	// 释放 Allocate 拿到的内存
	static void Deallocate(T* block);

	void Resize(int new_size);
};

// 分配原始内存：只申请空间，不构造任何元素。
// 用 operator new[] 而不是 new T[]，就是为了把"分配内存"和"构造对象"分开：
// 只有真正要用的槽位才做定位 new，这样 T 不必有默认构造函数，
// 而且元素的生命期我们能精确控制。
template <class T>
T* SeqList<T>::Allocate(int new_size)
{
	return static_cast<T*>(::operator new[](sizeof(T) * static_cast<size_t>(new_size)));
}

template <class T>
void SeqList<T>::Deallocate(T* block)
{
	::operator delete[](static_cast<void*>(block));
}

// 析构有效范围内的元素（下标 0 .. last_）
template <class T>
void SeqList<T>::DestroyAll()
{
	for (int i = 0; i <= last_; i++)
	{
		data_[i].~T();
	}
	last_ = -1;
}

template <class T>
SeqList<T>::SeqList(int sz)
{
	if (sz <= 0)
	{
		std::cerr << "无效的数组大小" << std::endl;
		exit(1);
	}
	max_size_ = sz;
	last_ = -1;
	data_ = Allocate(max_size_);   // 此时数组里还没有任何元素
}

template <class T>
SeqList<T>::SeqList(const SeqList<T>& other)
{
	max_size_ = other.max_size_;
	last_ = -1;
	data_ = Allocate(max_size_);

	// 对方有几个元素，就定位 new 几个（逐个拷贝构造）
	for (int i = 0; i < other.last_ + 1; i++)
	{
		new (data_ + i) T(other.data_[i]);
		last_++;
	}
}

template <class T>
SeqList<T>::~SeqList()
{
	DestroyAll();    // 先析构已构造的元素
	Deallocate(data_);   // 再释放内存
}

template <class T>
int SeqList<T>::Size() const
{
	return max_size_;
}

template <class T>
int SeqList<T>::Length() const
{
	return last_ + 1;
}

template <class T>
bool SeqList<T>::GetData(int i, T& x) const
{
	// last_ 是「最后一个元素的下标」（0 基），元素个数是 last_ + 1，
	// 所以第 i 个元素（1 基）存在 等价于 1 <= i <= last_ + 1。
	if (i >= 1 && i <= last_ + 1)
	{
		x = data_[i - 1];
		return true;
	}
	return false;
}

template <class T>
void SeqList<T>::SetData(int i, T& x)
{
	if (i >= 1 && i <= last_ + 1)
	{
		data_[i - 1] = x;
	}
}

template <class T>
bool SeqList<T>::Insert(int i, const T& x)
{
	if (i < 1 || i > last_ + 2)
	{
		return false;
	}
	if (last_ + 1 >= max_size_)
	{
		Resize(max_size_ * 2);
	}

	// n 是插入前的元素个数。插入位置是 i-1（1 基位序），要把它及后面的元素
	// 整体右移一位，再从 data_[n] 往前填：
	//   1) 空表（n == 0）：直接在 data_[0] 上定位 new 构造 x
	//   2) 非空：先右移 —— 从最后一个元素开始，
	//      把 data_[j] 搬到 data_[j+1]（j 从 n-1 降到 i-1）。
	//      data_[n] 是还没构造过的新槽位，而它是这里唯一的新位置：
	//      循环第一轮就把它构造出来（用 x 拷贝），之后每个位置都是赋值，
	//      所以对象个数从 n 变成 n+1，正好和有效长度一致。
	const int n = last_ + 1;
	if (n == 0)
	{
		new (data_) T(x);
	}
	else
	{
		new (data_ + n) T(x);          // 新槽位：先用 x 构造出来
		for (int j = n - 1; j >= i - 1; j--)
		{
			data_[j + 1] = data_[j];   // 往后挪一位（都是有效对象，赋值即可）
		}
		data_[i - 1] = x;              // 新元素放到插入位置
	}

	last_++;
	return true;
}

template <class T>
bool SeqList<T>::Remove(int i, T& x)
{
	// 删除和读取的范围相同：位序 1 .. last_+1 才有元素
	if (last_ == -1 || i < 1 || i > last_ + 1)
	{
		return false;
	}
	x = data_[i - 1];

	// 后面的元素逐个往左填一个位置。填完之后，下标 last_ 这一位的值
	// 已经被前一个元素覆盖过了，它虽然仍是"已构造"状态，但语义上已经没有用了，
	// 所以这里显式析构一次，保持"只有有效元素才处于构造状态"。
	for (int j = i - 1; j < last_; j++)
	{
		data_[j] = data_[j + 1];
	}
	data_[last_].~T();
	last_--;
	return true;
}

template <class T>
bool SeqList<T>::IsEmpty() const
{
	return (last_ == -1);
}

template <class T>
bool SeqList<T>::IsFull() const
{
	return (last_ + 1 >= max_size_);
}

template <class T>
void SeqList<T>::Resize(int new_size)
{
	if (new_size <= 0)
	{
		std::cerr << "无效的数组大小" << std::endl;
		exit(1);
	}
	if (new_size == max_size_)
	{
		return;
	}

	// 顺序不能颠倒：先把元素拷到新内存，再析构并释放旧内存
	T* new_array = Allocate(new_size);

	const int n = last_ + 1;
	for (int i = 0; i < n; i++)
	{
		new (new_array + i) T(data_[i]);   // 在新内存里逐个拷贝构造
	}

	DestroyAll();          // 析构旧内存里的元素
	Deallocate(data_);     // 释放旧内存

	data_ = new_array;
	max_size_ = new_size;
	last_ = n - 1;
}

template <class T>
int SeqList<T>::Search(const T& x) const
{
	for (int i = 0; i <= last_; i++)
	{
		if (data_[i] == x)
		{
			return i + 1;
		}
	}
	return 0;
}

template <class T>
int SeqList<T>::Locate(int i) const
{
	// 位序 1 .. last_+1 才有元素；越界返回 0
	if (i >= 1 && i <= last_ + 1)
	{
		return i;
	}
	return 0;
}

template <class T>
void SeqList<T>::Sort()
{
	for (int i = 0; i < last_; i++)
	{
		for (int j = 0; j < last_ - i; j++)
		{
			if (data_[j] > data_[j + 1])
			{
				std::swap(data_[j], data_[j + 1]);
			}
		}
	}
}

template <class T>
void SeqList<T>::Output()
{
	for (int i = 0; i <= last_; i++)
	{
		std::cout << std::left << std::setw(8) << data_[i];
	}
	std::cout << std::endl;
}

template <class T>
bool SeqList<T>::Delete(const T& x)
{
	// 双指针就地压缩：
	// j 是读指针，从 0 扫到 last_；k 是写指针，指向下一个要保留的位置。
	// 凡是与 x 相等的元素都跳过，不相等的写到 k 处。
	//
	// 关键：扫描期间绝不能析构元素。因为写指针 k 可能落在"被跳过"的位置上，
	// 而那些位置里的旧值还会被后面的元素读到（例如第 3 个元素被搬到 k 处时，
	// 读指针 j 仍要经过 k 之后的槽位）。一旦提前析构，右移就会把已失效的对象
	// 当成有效值搬走，导致数据错乱。
	// 所以等压缩完了，末尾多出来的那段 [k, last_] 再统一析构。
	int k = 0;
	for (int j = 0; j <= last_; j++)
	{
		if (!(data_[j] == x)) // 使用 !(data_[j] == x) 防止类型 T 没有 != 运算符
		{
			if (k != j)
			{
				data_[k] = data_[j];
			}
			k++;
		}
	}

	const bool deleted = (k != last_ + 1);
	for (int j = k; j <= last_; j++)
	{
		data_[j].~T();      // 这些位置已经没有有效元素了
	}
	last_ = k - 1;
	return deleted;
}

template <class T>
bool SeqList<T>::InsertAfter(const T& y, const T& x)
{
	// Search 返回 y 的 1 基位序，0 表示表中没有 y
	int i = Search(y);
	if (i == 0)
	{
		return false;
	}
	if (last_ + 1 >= max_size_)
	{
		Resize(max_size_ * 2);
	}
	// 复用 Insert：位序 i+1 即“第 i 个元素之后”
	return Insert(i + 1, x);
}

template <class T>
void SeqList<T>::Clear()
{
	// 逐个析构有效元素（让它们持有的资源立刻释放），数组空间留着复用
	DestroyAll();
}

template <class T>
SeqList<T>& SeqList<T>::operator=(const SeqList<T>& other)
{
	// 深拷贝：扩容后逐个赋值，多出来的元素要析构掉
	if (this == &other)
	{
		return *this;
	}
	if (max_size_ < other.max_size_)
	{
		Resize(other.max_size_);
	}

	const int old_n = last_ + 1;
	const int n = other.last_ + 1;
	const int common = (old_n < n) ? old_n : n;
	for (int i = 0; i < common; i++)
	{
		data_[i] = other.data_[i];
	}
	for (int i = common; i < n; i++)
	{
		new (data_ + i) T(other.data_[i]);
	}
	// 下标 n .. last_ 原来有元素，现在用不到了，逐个析构
	for (int i = n; i < old_n; i++)
	{
		data_[i].~T();
	}
	last_ = n - 1;
	return *this;
}
