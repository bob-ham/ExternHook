#include "../ExternHook.h"
#include "defs.h"
#include <string>
#include <tlhelp32.h>
#include <vector>






EH_STATUS __stdcall EH_Initialize(const char* processName)
{
	if (g_Lib.process) return EH_STATUS::EH_ERROR_INTENRAL;

	g_Lib.process = new Process(std::string(processName));

	if (g_Lib.process->get_pid() == 0)
	{
		delete g_Lib.process;
		g_Lib.process = nullptr;
		return EH_STATUS::EH_PROCESS_NOT_FOUND;
	}
	return EH_STATUS::EH_OK;
}

EH_STATUS __stdcall EH_CreateHook(void* jmpInstruction, void* newTarget)
{
	if (!g_Lib.process) return EH_STATUS::EH_ERROR_INTENRAL;

	DWORD targetPID = g_Lib.process->get_pid();

	g_Lib.process->SuspendAllThreads(targetPID, true);

	Hook h;
	bool success = h.hook(*g_Lib.process, jmpInstruction, newTarget);

	g_Lib.process->SuspendAllThreads(targetPID, false);

	if (!success)
		return EH_STATUS::EH_HOOK_FAILED;

	g_Lib.registry[jmpInstruction] = h;

	return EH_STATUS::EH_OK;
}

EH_STATUS __stdcall EH_CleanHook()
{
	if (!g_Lib.process) return EH_STATUS::EH_ERROR_INTENRAL;

	DWORD targetPID = g_Lib.process->get_pid();

	g_Lib.process->SuspendAllThreads(targetPID, true);

	/* Unhooking via a registry I made with std::map. */
	for (auto& [addr, hookObj] : g_Lib.registry)
	{
		hookObj.unhook(*g_Lib.process);
	}

	g_Lib.process->SuspendAllThreads(targetPID, false);
	
	g_Lib.registry.clear();
	return EH_STATUS::EH_OK;
}

EH_STATUS __stdcall EH_Map(const wchar_t* DllPath, uintptr_t* outEntryPoint)
{
	if (!g_Lib.process) return EH_STATUS::EH_PROCESS_NOT_FOUND;

	Mapper mapper(*g_Lib.process);
	g_Lib.lastMapResult = mapper.map(DllPath);

	if (!g_Lib.lastMapResult.success)
	{
		return EH_STATUS::EH_MAPPING_FAILED;
	}

	if (outEntryPoint)
	{
		*outEntryPoint = g_Lib.lastMapResult.entryPoint;
	}

	return EH_STATUS::EH_OK;
}

EH_STATUS __stdcall EH_Uninitialize()
{
	EH_CleanHook();
	delete g_Lib.process;
	g_Lib.process = nullptr;
	return EH_STATUS::EH_OK;
}