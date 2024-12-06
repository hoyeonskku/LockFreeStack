#pragma once
#include "Windows.h"
#include "CObjectPool.h"

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
	Node<T>(T value) : _value(value) {}
	~Node<T>() {}

public:
	Node<T>* _pNext = nullptr;
	T _value;
};


// 상위 17비트를 식별자로 사용
// 노드 메모리풀을 이용해야 크래시 나지 않음.
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
		pNewNodeValue = (Node<T>*)MAKE_VALUE(_id, pNewNode);
		do
		{
			pNewNode->_pNext = _pTopNodeValue;
			pNewNodeNextValue = pNewNode->_pNext;
		}
		// top이 저장한 값과 같은 경우에만 Push, 노드를 새로운 top으로 변경
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
		// top이 제거하려는 노드인 경우에만 Pop, next를 새로운 top으로 변경
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

