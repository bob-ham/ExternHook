/* test program */
#include <windows.h>
#include <iostream>
#include "../include/ExternHook.h"

const char* szProcessName = "RobloxPlayerBeta.exe";
const wchar_t* szDllPath = L"C:\\Users\\cheese\\source\\repos\\ExternHook\\x64\\Release\\exthk.dll";

int main()
{
	/* Get QPC as a test. */
	void* QPC = GetProcAddress(GetModuleHandleA("Kernel32.dll"), "QueryPerformanceCounter");
	EH_HOOK qpcHook = nullptr;

	if (EH_Initialize(szProcessName) != EH_OK)
	{
		printf("EH_Init failed!\n");
		return -1;
	}

	/* Map DLL into memory, then hook. */
	uintptr_t entry = 0;
	if (EH_Map(szDllPath, &entry) == EH_OK)
	{
		if (entry == 0)
		{
			printf("entry point is 0. check your exports\n");
			return -1;
		}

		/* Hook onto QPC. */
		EH_STATUS status = EH_CreateHook(QPC, (void*)entry, &qpcHook);
		if (status != EH_OK)
		{
			printf("EH_CreateHook failed!\n");
			EH_Uninitialize();
			return -1;
		}
	}
	else
	{
		printf("EH_Map failed!");
	}

	printf("Hook placed at address: 0x%p\n", (void*)entry);
	printf("Press any key to clean up.\n");
	Sleep(1000);
	EH_CleanHook(qpcHook);
	system("pause");

	EH_Uninitialize();

	return 0;
}