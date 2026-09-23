#pragma once

#include "LinearList.h"
template<class T>
class SeqList :LinearList<T>
{
protected:
	T* data_;
	int max_size_;
	int last;
	void Resize(int new_size);
	bool IndexIsValid(int i)
	{
		if (i > 0 && i <= last + 1)
		{
			return true;
		}
		else return false;
	}

public:
	SeqList(int sz = kDefaultSize);

	SeqList(SeqList<T>& other);

	~SeqList() { delete[] data_; }

	int Size()const { return max_size_ }

	int Length()const { return last + 1; }

	int Search(T& x)const;

	int Locate(int i)const;

	bool GetData(int i, T& x)const
	{
		if (IndexIsValid(i))
		{
			x = data_[i - 1];
			return true;
		}
		else return false;
	}

	void SetData(int i, T& x)
	{
		if (IndexIsValid(i))
		{
			data[i - 1] = x;
		}
	}

	bool Insert(int i, T& x);

	bool Remove(int i, T& x);

	bool IsEmpty()
	{
		return (last == -1) ? true : false;
	}

	bool IsFull()
	{
		return (last == max_size_ - 1) ? true : false;
	}

	void Input();

	void Output();

	SeqList<T> operator=(SeqList<T>& other);

};

template<class T>
SeqList<T>::SeqList(int size)
{
	if (size > 0)
	{
		max_size_ = size;
		last = -1;
		data = new T[max_size_];
		if (data_ == nullptr)
		{
			cerr << "存储分配错误" << endl;
			exit(1);
		}
	}
};

template<class T>
SeqList<T>::SeqList(SeqList<T>& other)
{
	max_size_ = other.Size();
	last = other.Length() - 1;
	T value;
	data = new T[max_size_];
	if (data_ == nullptr)
	{
		cerr << "存储分配错误" << endl;
		exit(1);
	}
	for (int i = 1; i <= last + 1; i++)
	{
		other.GetData(i, value);
		data[i - 1] = value;
	}
};

template<class T>
void SeqList<T>::Resize(int new_size)
{
	if (new_size <= 0)
	{
		cerr << "无效的数组大小" << endl;
		exit(1);
	}
	if (new_size != max_size_)
	{
		T* new_array = new T[new_size];
		if (new_array == nullptr)
		{
			cerr << "存储分配错误" << endl;
			exit(1);
		}
		int n = last + 1;
		T* srcptr = data_;
		T* desptr = newarray;
		while (n--)
			*desptr++ = *srcptr++;
		delete[] data_;
		data = nrearray;
		max_size_ = new_size;
	}
}

template<class T>
int SeqList<T>::Search(T& x)const
{

}