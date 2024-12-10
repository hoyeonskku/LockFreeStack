#include <iostream>
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
	TestStruct()
	{
		a = 11;
	}
	int a = 10;
};
//CMemoryPool<TestStruct, true> g_pool(10000);

unsigned long long g_data = 0;
CLockFreeStack<unsigned long long> g_Stack;
//CLockFreeStack<TestStruct> g_Stack1;

unsigned long long arr[10001];

unsigned int __stdcall ThreadFunc(void* arg)
{
	/*while (!boolean1)
	{
	}*/
	{

		for (int i = 0; i < 2000; i++)
		{
			int data = InterlockedIncrement(&g_data);
			g_Stack.Push(data);
		}

		for (int i = 0; i < 2000; i++)
		{
			unsigned long long a;
			g_Stack.Pop(a);
			arr[a]++;
			if (arr[a] != 1)
				DebugBreak();
		}

	}

	return true;
}
//unsigned int __stdcall ThreadFunc(void* arg)
//{
//	TestStruct* arr[10];
//
//	while (true)
//	{
//		for (int i = 0; i < 10; i++)
//		{
//			arr[i] = g_pool.Alloc();
//			arr[i]->a++;
//		}
//
//		for (int i = 0; i < 10; i++)
//		{
//			if (arr[i]->a != 11)
//				DebugBreak();
//
//			arr[i]->a--;
//		}
//
//		for (int i = 0; i < 10; i++)
//		{
//			if (arr[i]->a != 10)
//				DebugBreak();
//			g_pool.Free(arr[i]);
//		}
//	}
//
//	return 0;
//}

//unsigned int __stdcall ThreadFunc(void* arg)
//{
//	TestStruct* arr[10];
//
//	while (true)
//	{
//		for (int i = 0; i < 10; i++)
//		{
//			arr[i] = g_pool.Alloc();
//			arr[i]->a++;
//		}
//
//		for (int i = 0; i < 10; i++)
//		{
//			if (arr[i]->a != 11)
//				DebugBreak();
//
//			arr[i]->a--;
//		}
//
//		for (int i = 0; i < 10; i++)
//		{
//			if (arr[i]->a != 10)
//				DebugBreak();
//			g_pool.Free(arr[i]);
//		}
//	}
//
//	return 0;
//}

int main()
{
	TestStruct t;

	//g_Stack1.Push(t);

	HANDLE _handle[5];
	for (int i = 0; i < 5; i++)
	{
		_handle[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, NULL, nullptr);
	}
	WaitForMultipleObjects(5, _handle, true, INFINITE);

	for (int i = 1; i < 10001; i++)
	{
		if (arr[i] != 1)
			DebugBreak();
	}
}

