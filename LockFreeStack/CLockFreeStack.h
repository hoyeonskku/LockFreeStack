#pragma once
#include "Windows.h"

#define QueueSize 5000

enum EventType
{
	push,
	pop,
};

template <typename T>
class Node
{
public:
	Node<T>(T value) : _value(value){}
	~Node<T>() {}

public:
	Node<T>* _pNext = nullptr;
	T _value;
};


// ���� 17��Ʈ�� �ĺ��ڷ� ���
// ��� �޸�Ǯ�� �̿��ؾ� ũ���� ���� ����.
// value = id(17Bit) + pNode(47Bit)
template <typename T>
class CLockFreeStack
{	
public:
	 void Push(T data)
	 {
		 Node<T>* pNewNode;
		 Node<T>* pNewNodeValue;
		 pNewNode = new Node<T>(data);
		 unsigned long long id = InterlockedIncrement(&_id);
		 pNewNodeValue = (Node<T>*)((id << 47) | (unsigned long long) pNewNode);
		 do 
		 {
			 pNewNode->_pNext = _pTopNodeValue;
		 }
		 // top�� ������ ���� ���� ��쿡�� Push, ��带 ���ο� top���� ����
		 while ((Node<T>*) InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pNewNodeValue, (unsigned long long) pNewNode->_pNext) != pNewNode->_pNext);
	 }

	 T& Pop()
	 {
		 Node<T>* pReleaseNode;
		 Node<T>* pReleaseNodeValue;
		 unsigned long long releaseNodeId;
		 do
		 {
			 pReleaseNodeValue = _pTopNodeValue;
			 // ��¼�� �»��� ID�̱⿡ & 0x1FFFF �����൵ ��.
			 releaseNodeId = (unsigned long long)pReleaseNodeValue >> 47;
			 pReleaseNode = (Node<T>*) ((unsigned long long)pReleaseNodeValue & (unsigned long long) 0x7FFFFFFFFFFF);
			 /*if (!Release)
				 return;*/
		 }
		 // top�� �����Ϸ��� ����� ��쿡�� Pop, next�� ���ο� top���� ����
		 while ((Node<T>*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNode->_pNext, (unsigned long long)pReleaseNodeValue) != pReleaseNodeValue);
		 T& retval = pReleaseNode->_value;
		 delete pReleaseNode;
		 return retval;
	 }

private:
	Node<T>* _pTopNodeValue = nullptr;
	unsigned long long _id = 0;
};

