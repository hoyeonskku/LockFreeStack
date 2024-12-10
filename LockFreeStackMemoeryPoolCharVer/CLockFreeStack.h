#pragma once
#include "Windows.h"
#include "CObjectPool.h"

#define QueueSize 5000

enum StackEventType
{
	push,
	pop,
};



template <typename T>
class Node
{
public:
	Node<T>(T value) : _value(value) {}
	Node<T>() {}
	~Node<T>() {}

public:
	alignas(sizeof(T)) char _value[sizeof(T)];
	Node<T>* _pNextValue = nullptr;
};

template <typename T>
class LogClass
{
public:
	StackEventType _type;
	Node<T>* _pCurrent = nullptr;
	Node<T>* _pNext = nullptr;
};

template <typename T>
class LogQueue
{
public:
	LogClass<T> _arr[10000];
	unsigned long long _count;

	void Enqueue(LogClass<T>& logClass)
	{
		unsigned long long index = InterlockedIncrement(&_count) % 10000;
		_arr[index] = logClass;
	}
};

// ���� 17��Ʈ�� �ĺ��ڷ� ���
// ��� �޸�Ǯ�� �̿��ؾ� ũ���� ���� ����.
// value = id(17Bit) + pNode(47Bit)
template <typename T>
class CLockFreeStack
{
public:
	CLockFreeStack() : _pool(0)
	{

	}
	~CLockFreeStack()
	{

	}

	void Push(T data)
	{
		Node<T>* pNewNode;
		Node<T>* pNewNodeValue;
		Node<T>* pNewNodeNextValue;
		pNewNode = _pool.Alloc();
		new ((char*)pNewNode + offsetof(Node<T>, _value)) T(data);
		pNewNodeValue = (Node<T>*)MAKE_VALUE(_id, pNewNode);
		do
		{
			pNewNodeNextValue = _pTopNodeValue;
			pNewNode->_pNextValue = pNewNodeNextValue;
		}
		// top�� ������ ���� ���� ��쿡�� Push, ��带 ���ο� top���� ����
		while ((Node<T>*) InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pNewNodeValue, (unsigned long long) pNewNodeNextValue) != pNewNodeNextValue);

		//LogClass<T> logClass;
		//logClass._pCurrent = (Node<T>*)MAKE_NODE(pNewNodeValue);
		//logClass._pNext = (Node<T>*)MAKE_NODE(pNewNodeNextValue);
		//logClass._type = StackEventType::push;
		//_logQueue.Enqueue(logClass);
	}

	bool Pop(T& data)
	{
		Node<T>* pReleaseNode;
		Node<T>* pReleaseNodeValue;
		Node<T>* pReleaseNodeNextValue;
		do
		{
			pReleaseNodeValue = _pTopNodeValue;
			pReleaseNode = (Node<T>*) MAKE_NODE(pReleaseNodeValue);
			if (pReleaseNodeValue == nullptr)
				DebugBreak();
			pReleaseNodeNextValue = pReleaseNode->_pNextValue;
		}
		// top�� �����Ϸ��� ����� ��쿡�� Pop, next�� ���ο� top���� ����
		while ((Node<T>*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNodeNextValue, (unsigned long long)pReleaseNodeValue) != pReleaseNodeValue);

		memcpy((void*)&data, pReleaseNode->_value, sizeof(T));
		//LogClass<T> logClass;
		//logClass._pCurrent = (Node<T>*) MAKE_NODE(pReleaseNodeValue);
		//logClass._pNext = (Node<T>*)MAKE_NODE(pReleaseNodeNextValue);
		//logClass._type = StackEventType::pop;
		//_logQueue.Enqueue(logClass);

		_pool.Free(pReleaseNode);
		return true;
	}
	
	bool TryPop(T& data)
	{
		Node<T>* pReleaseNode;
		Node<T>* pReleaseNodeValue;
		Node<T>* pReleaseNodeNextValue;
		if ((Node<T>*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNodeNextValue, (unsigned long long)pReleaseNodeValue) == pReleaseNodeValue);
		
		{
			pReleaseNodeValue = _pTopNodeValue;
			pReleaseNode = (Node<T>*) MAKE_NODE(pReleaseNodeValue);
			if (pReleaseNodeValue == nullptr)
				DebugBreak();
			pReleaseNodeNextValue = pReleaseNode->_pNextValue;
			// top�� �����Ϸ��� ����� ��쿡�� Pop, next�� ���ο� top���� ����

			memcpy((void*)&data, pReleaseNode->_value, sizeof(T));
			//LogClass<T> logClass;
			//logClass._pCurrent = (Node<T>*) MAKE_NODE(pReleaseNodeValue);
			//logClass._pNext = (Node<T>*)MAKE_NODE(pReleaseNodeNextValue);
			//logClass._type = StackEventType::pop;
			//_logQueue.Enqueue(logClass);

			_pool.Free(pReleaseNode);
			return true;
		}

		return false;
	}

private:
	CMemoryPool<Node<T>> _pool;
	Node<T>* _pTopNodeValue = nullptr;
	unsigned long long _id = 0;
	LogQueue<T> _logQueue;
};

