#pragma once
#include "Windows.h"
#include "CObjectPool.h"

#define QueueSize 5000
#define MAKE_NODE(value) ((unsigned long long)pReleaseNodeValue & (unsigned long long) 0x7FFFFFFFFFFF)
#define MAKE_VALUE(id, node) ((InterlockedIncrement(&_id) << 47) | (unsigned long long) pNewNode)
enum EventType
{
	push,
	pop,
};

template <typename T>
class Node
{
public:
	Node<T>(T value) : _value(value) {}
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
	CLockFreeStack() : _pool(0) { }
	~CLockFreeStack() { }

	void Push(T data)
	{
		Node<T>* pNewNode;
		Node<T>* pNewNodeValue;
		Node<T>* pNewNodeNextValue;
		pNewNode = _pool.Alloc();
		pNewNode->_value = data;
		pNewNodeValue = (Node<T>*)((InterlockedIncrement(&_id) << 47) | (unsigned long long) pNewNode);
		do
		{
			pNewNode->_pNext = _pTopNodeValue;
			pNewNodeNextValue = pNewNode->_pNext;
		}
		// top�� ������ ���� ���� ��쿡�� Push, ��带 ���ο� top���� ����
		while ((Node<T>*) InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pNewNodeValue, (unsigned long long) pNewNodeNextValue) != pNewNodeNextValue);
	}

	T& Pop()
	{
		Node<T>* pReleaseNode;
		Node<T>* pReleaseNodeValue;
		do
		{
			pReleaseNodeValue = _pTopNodeValue;
			pReleaseNode = (Node<T>*) MAKE_NODE(pReleaseNodeValue);
			/*if (!Release)
				return;*/
		}
		// top�� �����Ϸ��� ����� ��쿡�� Pop, next�� ���ο� top���� ����
		while ((Node<T>*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNode->_pNext, (unsigned long long)pReleaseNodeValue) != pReleaseNodeValue);
		T& retval = pReleaseNode->_value;
		_pool.Free(pReleaseNode);
		return retval;
	}

private:
	CMemoryPool<Node<T>> _pool;
	Node<T>* _pTopNodeValue = nullptr;
	unsigned long long _id = 0;
};

