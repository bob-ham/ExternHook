/* handle hijacking code */
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "ntapi.h"

class Process
{
private:
	std::string name;
	DWORD pid;
	HANDLE handle;

	HANDLE hijack_handle(DWORD pid);
	DWORD GetPID(std::string name);

	pNtReadVirtualMemory    _NtReadVirtualMemory;
	pNtWriteVirtualMemory   _NtWriteVirtualMemory;
	pNtProtectVirtualMemory _NtProtectVirtualMemory;
	pNtAllocateVirtualMemory _NtAllocateVirtualMemory;
	pNtOpenProcess           _NtOpenProcess;
	pNtClose                _NtClose;
	pNtDuplicateObject      _NtDuplicateObject;


public:
	Process(std::string name);
	~Process();
	Process(const Process& other) = delete;
	Process& operator=(const Process& other) = delete;
	Process(Process&& other) noexcept;
	Process& operator=(Process&& other) noexcept;

	DWORD get_pid();
	HANDLE get_handle();

	/* helper functions */
	NTSTATUS read_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read);
	NTSTATUS write_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T written);
	NTSTATUS change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt);
	PVOID allocate_mem(SIZE_T size, ULONG type, ULONG prot);

	/* temp helper function to just see if thisll work or not */
	void SuspendAllThreads(DWORD processID, bool suspend);


	/* module stuff */
	struct Module {
		std::string name;
		BYTE* base;
		int size;
	};

	std::vector<Module> get_all_modules();
	Module* GetModuleByName(const std::vector<Module>& modules, const std::string& name);
};

