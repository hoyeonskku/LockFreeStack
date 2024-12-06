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
		st_BLOCK_NODE* _pNext;          // 다음 블럭 노드 포인터
		CMemoryPool<DATA, bPlacementNew>* pParent;        // 부모 블럭 노드 포인터
		char guardBefore[8];
		DATA _data;                       // T 타입의 데이터
		char guardAfter[8];
#endif
#ifndef _DEBUG
		st_BLOCK_NODE* _pNext;          // 다음 블럭 노드 포인터
		DATA _data;                       // T 타입의 데이터
#endif
	};
public:
#ifdef _DEBUG		
	static const long long GUARD_VALUE = 0xEAEAEAEAEAEAEAEA;

#endif
	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CMemoryPool(int iBlockNum) : _pTopNodeValue(nullptr)
#ifdef _DEBUG
		, m_iUseCount(0), m_iCapacity(iBlockNum)
#endif
	{

#ifdef _DEBUG
		// 블록 초기화
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
		// 블록 초기화
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
				temp->_data.~DATA();  // 메모리 해제
			}

			delete (st_BLOCK_NODE*)((unsigned long long) temp);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
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
			// 어쩌피 태생이 ID이기에 & 0x1FFFF 안해줘도 됨.
			releaseNodeId = (unsigned long long)pReleaseNodeValue >> 47;
			pReleaseNode = (st_BLOCK_NODE*)((unsigned long long)pReleaseNodeValue & (unsigned long long) 0x7FFFFFFFFFFF);
			/*if (!Release)
				return;*/
		}
		// top이 제거하려는 노드인 경우에만 Pop, next를 새로운 top으로 변경
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
			// 어쩌피 태생이 ID이기에 & 0x1FFFF 안해줘도 됨.
			releaseNodeId = EXTRACT_ID(pReleaseNodeValue);
			pReleaseNode = (st_BLOCK_NODE*)MAKE_NODE(pReleaseNodeValue);
			/*if (!Release)
				return;*/
		}
		// top이 제거하려는 노드인 경우에만 Pop, next를 새로운 top으로 변경
		while ((st_BLOCK_NODE*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pReleaseNode->_pNext, (unsigned long long)pReleaseNodeValue) != pReleaseNodeValue);
		return (DATA*)((unsigned long long)((char*)pReleaseNode + offsetof(st_BLOCK_NODE, _data)));
#endif
	};

	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA* pData)
	{
#ifdef _DEBUG			

		st_BLOCK_NODE* pBlockNode = (st_BLOCK_NODE*)((char*)pData - offsetof(st_BLOCK_NODE, _data));
		st_BLOCK_NODE* pBlockValue = (st_BLOCK_NODE*)((unsigned long long)pBlockNode | (InterlockedIncrement(&_id) << 47));
		if (*(long long*)(pBlockNode->guardBefore) != GUARD_VALUE || *(long long*)(pBlockNode->guardAfter) != GUARD_VALUE)
		{
			// 가드 값이 손상되었음을 보고하거나 처리
			std::cerr << "Memory corruption detected during Free!" << std::endl;
			//				std::abort();
			return false; // 또는 적절한 오류 처리를 추가
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
		// top이 저장한 값과 같은 경우에만 Push, 노드를 새로운 top으로 변경
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
		// top이 저장한 값과 같은 경우에만 Push, 노드를 새로운 top으로 변경
		while ((st_BLOCK_NODE*)InterlockedCompareExchange((unsigned long long*) & _pTopNodeValue, (unsigned long long) pBlockValue, (unsigned long long) pBlockNode->_pNext) != pBlockNode->_pNext);
		return true;
#endif
	};
#ifdef _DEBUG
	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetCapacityCount(void) { return m_iCapacity; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
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