#pragma once

const int kInitialMaxCapacity = 16;
enum bool{ false, true };
template<class T>
class LinearList
{
public:
	LinearList();
	virtual ~LinearList();
	virtual int Size()const = 0;
	virtual int Search(T& x)const = 0;
	virtual int Locate(int i)const = 0;
	virtual bool GetData(int i, T& x)const = 0;
	virtual void SetData(int i, T& x)const = 0;
	virtual bool Insert(int i, T& x)const = 0;
	virtual bool Remove(int ii, T& x)const = 0:
	virtual bool IsEmpty()const = 0;
	virtual bool IsFull()const = 0;
	virtual void Sort() = 0;
	virtual void Input() = 0;
	virtual void Output() = 0;
	virtual LinearList<T> operator=(LinearList<T>& other) = 0;

protected:
	T data_;
	int length_;
};