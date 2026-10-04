#pragma once

template<class T>
class LinearList
{
public:
	LinearList();
	virtual ~LinearList();
	virtual int Size() const = 0;
	virtual int Search(const T& x) const = 0;
	virtual int Locate(int i) const = 0;
	virtual bool GetData(int i, T& x) const = 0;
	virtual void SetData(int i, T& x) = 0;
	virtual bool Insert(int i, const T& x) = 0;
	virtual bool Remove(int i, T& x) = 0;
	virtual bool IsEmpty() const = 0;
	virtual bool IsFull() const = 0;
	virtual void Sort() = 0;
	virtual void Output() = 0;

	// 按元素值删除：删除表中所有值等于 x 的节点（补充需求 1）
	virtual bool Delete(const T& x) = 0;

	// 按元素值插入：在第一个值等于 y 的节点之后插入 x；
	// 表中不存在 y 时不插入，返回 false（需求 2）
	virtual bool InsertAfter(const T& y, const T& x) = 0;

	// 清空表（析构前把资源释放干净）
	virtual void Clear() = 0;
};

template <class T>
LinearList<T>::LinearList()
{
}

template <class T>
LinearList<T>::~LinearList()
{
}
