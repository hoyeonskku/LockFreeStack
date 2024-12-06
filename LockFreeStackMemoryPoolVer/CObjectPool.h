#ifndef  __MEMORY_POOL__
#define  __MEMORY_POOL__
#pragma once


#include <new.h>
#include <iostream>
#include <stack>
#include <assert.h>
#include <Windows.h>
#define MAKE_NODE(value) ((unsigned long long)value & 0x7FFFFFFFFFFF)
#define MAKE_VALUE(id, node) ((InterlockedIncrement(&id) << 47) | (unsigned long long) node)
#define EXTRACT_ID(value) (unsigned long long)value >> 47;

template <class DATA, bool bPlacementNew = false>
class CMemoryPool
{
public:
	struct st_BLOCK_NODE
	{
#ifdef _DEBUG
		st_BLOCK_NODE* _pNext;          // ���� �� ��� ������
		CMemoryPool<DATA, bPlacementNew>* pParent;        // �θ� �� ��� ������
		char guardBefore[8];
		DATA _data;                       // T Ÿ���� ������
		char guardAfter[8];
#endif
#ifndef _DEBUG
		st_BLOCK_NODE* _pNext;          // ���� �� ��� ������
		DATA _data;                       // T Ÿ���� ������
#endif
	};
public:
#ifdef _DEBUG		
	static const long long GUARD_VALUE = 0xEAEAEAEAEAEAEAEA;

#endif
	//////////////////////////////////////////////////////////////////////////
	// ������, �ı���.
	//
	// Parameters:	(int) �ʱ� �� ����.
	//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CMemoryPool(int iBlockNum) : _pTopNodeValue(nullptr)
#ifdef _DEBUG
		, m_iUseCount(0), m_iCapacity(iBlockNum)
#endif
	{

#ifdef _DEBUG
		// ��� �ʱ�ȭ
		st_BLOCK_NODE* newNode;
		for (int i = 0; i < iBlockNum; i++)
		{
			newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			if constexpr (bPlacementNew)
			{
				new ((char*)newNode + offsetof(st_BLOCK_NODE, _data)) DATA();
			}
			newNode->pParent = this;
			newNode->_pNext = _pTopNodeValue;

			_pTopNodeValue = (st_BLOCK_NODE*)((unsigned long long) newNode | ((_id++) << 47));

			*((long long*)newNode->guardBefore) = GUARD_VALUE;
			*((long long*)newNode->guardAfter) = GUARD_VALUE;
			m_iCapacity++;
			m_iUseCount++;
		}
#endif

#ifndef _DEBUG
		// ��� �ʱ�ȭ
		st_BLOCK_NODE* newNode;
		for (int i = 0; i < iBlockNum; i++)
		{
			newNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));

			if constexpr (bPlacementNew)
			{
				new ((char*)newNode + offsetof(st_BLOCK_NODE, _data)) DATA();
			}
			newNode->_pNext = _pTopNodeValue;
			_pTopNodeValue = (st_BLOCK_NODE*)MAKE_VALUE(_id, newNode);
		}
#endif
	};

	~CMemoryPool()
	{
		while (_pTopNodeValue)
		{
			st_BLOCK_NODE* temp = (st_BLOCK_NODE*)MAKE_NODE(_pTopNodeValue);

			_pTopNodeValue = temp->_pNext;
			if constexpr (bPlacementNew)
			{
				temp->_data.~DATA();  // �޸� ����
			}

			delete (st_BLOCK_NODE*)((unsigned long long) temp);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// �� �ϳ��� �Ҵ�޴´�.  
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
	//////////////////////////////////////////////////////////////////////////
	DATA* Alloc(void)
	{
#ifdef _DEBUG

		st_BLOCK_NODE* pReleaseNode;
		st_BLOCK_NODE* pReleaseNodeValue;
		unsigned long long releaseNodeId;
		if (_pTopNodeValue == nullptr)
		{
			pReleaseNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			pReleaseNode->pParent = this;
			*((long long*)pReleaseNode->guardBefore) = GUARD_VALUE;
			*((long long*)pReleaseNode->guardAfter) = GUARD_VALUE;
			if constexpr (bPlacementNew == true)
			{
				new ((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data)) DATA;
			}
			return (DATA*)((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data));
		}
		do
		{
			pReleaseNodeValue = _pTopNodeValue;
			// ��¼�� �»��� ID�̱⿡ & 0x1FFFF �����൵ ��.
			releaseNodeId = (unsigned long long)pReleaseNodeValue >> 47;
			pReleaseNode = (st_BLOCK_NODE*)((unsigned long long)pReleaseNodeValue & (unsigned long long) 0x7FFFFFFFFFFF);
			/*if (!Release)
				return;*/
		}
		// top�� �����Ϸ��� ����� ��쿡�� Pop, next�� ���ο� top���� ����
		while ((st_BLOCK_NODE*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNode->_pNext, (unsigned long long)pReleaseNodeValue) != pReleaseNodeValue);

		m_iCapacity++;
		m_iUseCount++;
		return (DATA*)((unsigned long long)((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data)));
#endif
#ifndef _DEBUG
		st_BLOCK_NODE* pReleaseNode;
		st_BLOCK_NODE* pReleaseNodeValue;
		unsigned long long releaseNodeId;
		if (_pTopNodeValue == nullptr)
		{
			pReleaseNode = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			if constexpr (bPlacementNew == true)
			{
				new ((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data)) DATA;
			}
			return (DATA*)((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data));
		}
		do
		{
			pReleaseNodeValue = _pTopNodeValue;
			// ��¼�� �»��� ID�̱⿡ & 0x1FFFF �����൵ ��.
			releaseNodeId = EXTRACT_ID(pReleaseNodeValue);
			pReleaseNode = (st_BLOCK_NODE*)MAKE_NODE(pReleaseNodeValue);
			/*if (!Release)
				return;*/
		}
		// top�� �����Ϸ��� ����� ��쿡�� Pop, next�� ���ο� top���� ����
		while ((st_BLOCK_NODE*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNode->_pNext, (unsigned long long)pReleaseNodeValue) != pReleaseNodeValue);
		return (DATA*)((unsigned long long)((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data)));
#endif
	};

	//////////////////////////////////////////////////////////////////////////
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters: (DATA *) �� ������.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA* pData)
	{
#ifdef _DEBUG			

		st_BLOCK_NODE* pBlockNode = (st_BLOCK_NODE*)((char*)pData - offsetof(st_BLOCK_NODE, _data));
		st_BLOCK_NODE* pBlockValue = (st_BLOCK_NODE*)((unsigned long long)pBlockNode | (InterlockedIncrement(&_id) << 47));
		if (*(long long*)(pBlockNode->guardBefore) != GUARD_VALUE || *(long long*)(pBlockNode->guardAfter) != GUARD_VALUE)
		{
			// ���� ���� �ջ�Ǿ����� �����ϰų� ó��
			std::cerr << "Memory corruption detected during Free!" << std::endl;
			//				std::abort();
			return false; // �Ǵ� ������ ���� ó���� �߰�
		}
		if (pBlockNode->pParent != this)
		{
			return false;
		}
		if constexpr (bPlacementNew == true)
		{
			pBlockNode->_data.~DATA();
		}
		do
		{
			pBlockNode->_pNext = _pTopNodeValue;
		}
		// top�� ������ ���� ���� ��쿡�� Push, ��带 ���ο� top���� ����
		while ((st_BLOCK_NODE*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pBlockValue, (unsigned long long) pBlockNode->_pNext) != pBlockNode->_pNext);
		m_iUseCount--;
		return true;


#endif
#ifndef _DEBUG
		st_BLOCK_NODE* pBlockNode = (st_BLOCK_NODE*)((char*)pData - offsetof(st_BLOCK_NODE, _data));
		st_BLOCK_NODE* pBlockValue = (st_BLOCK_NODE*)MAKE_VALUE(_id, pBlockNode);
		if constexpr (bPlacementNew == true)
		{
			pBlockNode->_data.~DATA();
		}
		do
		{
			pBlockNode->_pNext = _pTopNodeValue;
		}
		// top�� ������ ���� ���� ��쿡�� Push, ��带 ���ο� top���� ����
		while ((st_BLOCK_NODE*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pBlockValue, (unsigned long long) pBlockNode->_pNext) != pBlockNode->_pNext);
		return true;
#endif
	};
#ifdef _DEBUG
	//////////////////////////////////////////////////////////////////////////
	// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	//
	// Parameters: ����.
	// Return: (int) �޸� Ǯ ���� ��ü ����
	//////////////////////////////////////////////////////////////////////////
	int		GetCapacityCount(void) { return m_iCapacity; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �� ������ ��´�.
	//
	// Parameters: ����.
	// Return: (int) ������� �� ����.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return m_iUseCount; }
#endif

private:
	st_BLOCK_NODE* _pTopNodeValue = nullptr;
	unsigned long long _id = 0;
#ifdef _DEBUG
	unsigned int m_iUseCount;
	unsigned int m_iCapacity;
#endif
};

#endif