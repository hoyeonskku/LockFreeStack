#include <iostream>
#include "CLockFreeStack.h"
#include <thread>

unsigned int g_data = 0;
unsigned int boolean1 = 0;
CLockFreeStack<int> g_Stack;

unsigned int __stdcall ThreadFunc(void* arg)
{
	/*while (!boolean1)
	{
	}*/
	{
		while (1)
		{
		int data = InterlockedIncrement(&g_data);
			for (int i = 0; i < 5; i++)
				g_Stack.Push(data);

			for (int i = 0; i < 5; i++)
				g_Stack.Pop();
		}
	}
}

int main()
{
	HANDLE _handle[2];

	_handle[0] = (HANDLE) _beginthreadex(NULL, 0, ThreadFunc, nullptr, CREATE_SUSPENDED, nullptr);
	_handle[1] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, CREATE_SUSPENDED, nullptr);

	ResumeThread(_handle[0]);
	ResumeThread(_handle[1]);
//_beginthreadex(NULL, 0, ThreadFunc, nullptr, 0, nullptr);
	//_beginthreadex(NULL, 0, ThreadFunc, nullptr, 0, nullptr);

	while (true)
	{
		
	}
}

