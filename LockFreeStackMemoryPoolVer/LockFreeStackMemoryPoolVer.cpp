﻿#include <iostream>
#include "CLockFreeStack.h"
#include <thread>

// 원하는만큼 할당됐는지?
// 원하는만큼 반환됐는지?
// 똑같은 데이터를 여러 스레드가 얼록해갔는가?
// 만개를 alloc 했을때 다 성공했는지? , 실패가 있는지 확인,
// 메모리풀에 전부 반환됐는지 확인.
// 그래서? 
// 할당받은 데이터 변화를 확인하자.

struct TestStruct
{
	int a = 10;
};
CMemoryPool<TestStruct, true> g_pool(10000);



unsigned int __stdcall ThreadFunc(void* arg)
{
	TestStruct* arr[1000];

	for (int i = 0; i < 1000; i++)
	{
		arr[i] = g_pool.Alloc();
		arr[i]->a++;
	}

	for (int i = 0; i < 1000; i++)
	{
		if (arr[i]->a == 10)
			DebugBreak();

		arr[i]->a--;
	}
	
	for (int i = 0; i < 1000; i++)
	{
		if (arr[i]->a != 10)
			DebugBreak();
		g_pool.Free(arr[i]);
	}

	return 0;
}

int main()
{
	HANDLE _handle[4];
	while (true)
	{
		_handle[0] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, NULL, nullptr);
		_handle[1] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, NULL, nullptr);
		_handle[2] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, NULL, nullptr);
		_handle[3] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, NULL, nullptr);

		WaitForMultipleObjects(4, _handle, true, INFINITE);
	}
}

