#include "LinearList.h"

template<class T>
struct Node
{
	T data_;
	Node* next_;
};

template<class T>
class LinkedList:public LinearList
{
public:
	LinkedList();
	LinkedList(LinkedList* other);
	~LinkedList();

private:
	Node* header;
};