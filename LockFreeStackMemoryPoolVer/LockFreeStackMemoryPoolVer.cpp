#include <iostream>
#include "CLockFreeStack.h"
#include <thread>

unsigned int g_data = 0;
unsigned int boolean1 = 0;
CLockFreeStack<int> g_Stack;

unsigned int __stdcall ThreadFunc(void* arg)
{
	{
		while (1)
		{
			for (int i = 0; i < 10000; i++)
			{
			int data = InterlockedIncrement(&g_data);
				g_Stack.Push(data);
			}

			for (int i = 0; i < 10000; i++)
			{
				int data = InterlockedDecrement(&g_data);
				std::cout << g_Stack.Pop() << std::endl;
			}
		}
	}
}

int main()
{
	HANDLE _handle[2];

	_handle[0] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, CREATE_SUSPENDED, nullptr);
	_handle[1] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, nullptr, CREATE_SUSPENDED, nullptr);

	ResumeThread(_handle[0]);
	ResumeThread(_handle[1]);

	while (true)
	{

	}
}

