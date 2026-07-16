#include "../ExternHook.h"
#include "defs.h"
#include <string>
#include <tlhelp32.h>
#include <vector>


EH_STATUS __stdcall EH_Initialize(const char* processName)
{
	if (g_Lib.process) return EH_ERROR_INTENRAL;

	g_Lib.process = new Process(std::string(processName));

	if (g_Lib.process->get_pid() == 0)
	{
		delete g_Lib.process;
		g_Lib.process = nullptr;
		return EH_PROCESS_NOT_FOUND;
	}
	return EH_STATUS::EH_OK;
}

EH_STATUS __stdcall EH_CreateHook(void* jmpInstruction, void* newTarget, EH_HOOK* outHook)
{
	if (!g_Lib.process) return EH_ERROR_INTENRAL;
	if (g_Lib.registry.find(jmpInstruction) != g_Lib.registry.end()) return EH_HOOK_FAILED;

	DWORD targetPID = g_Lib.process->get_pid();

	g_Lib.process->SuspendAllThreads(targetPID, true);

	Hook* h = new Hook();
	bool success = h->hook(*g_Lib.process, jmpInstruction, newTarget);

	g_Lib.process->SuspendAllThreads(targetPID, false);

	if (!success)
	{
		delete h;
		return EH_HOOK_FAILED;
	}

	g_Lib.registry[jmpInstruction] = h;
	*outHook = (EH_HOOK)h;

	return EH_STATUS::EH_OK;
}

EH_STATUS __stdcall EH_CleanHook(EH_HOOK hook)
{
	if (!g_Lib.process) return EH_ERROR_INTENRAL;

	DWORD targetPID = g_Lib.process->get_pid();
	g_Lib.process->SuspendAllThreads(targetPID, true);
	if (hook != nullptr)
	{
		Hook* h = (Hook*)hook;
		h->unhook(*g_Lib.process);

		for (auto it = g_Lib.registry.begin(); it != g_Lib.registry.end(); ++it)
		{
			if (it->second == h)
			{
				g_Lib.registry.erase(it);
				break;
			}
		}
		delete h;
	}
	else
	{
		for (auto& [addr, hookObj] : g_Lib.registry)
		{
			hookObj->unhook(*g_Lib.process);
			delete hookObj;
		}
		g_Lib.registry.clear();
	}

	g_Lib.process->SuspendAllThreads(targetPID, false);
	return EH_OK;
}

EH_STATUS __stdcall EH_Map(const wchar_t* DllPath, uintptr_t* outEntryPoint)
{
	if (!g_Lib.process) return EH_PROCESS_NOT_FOUND;

	Mapper mapper(*g_Lib.process);
	g_Lib.lastMapResult = mapper.map(DllPath);

	if (!g_Lib.lastMapResult.success)
	{
		return EH_MAPPING_FAILED;
	}

	if (outEntryPoint)
	{
		*outEntryPoint = g_Lib.lastMapResult.entryPoint;
	}

	return EH_OK;
}

EH_STATUS __stdcall EH_Uninitialize()
{
	delete g_Lib.process;
	g_Lib.process = nullptr;
	return EH_OK;
}
