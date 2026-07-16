#include "process.h"
#include "ntapi.h"
#include <string>
#include <tlhelp32.h>
#include <iostream>

HANDLE Process::get_handle() { return this->handle; }

DWORD Process::GetPID(std::string name)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return 0;

	PROCESSENTRY32W PE;
	PE.dwSize = sizeof(PROCESSENTRY32W);
	std::wstring wname(name.begin(), name.end());

	if (Process32FirstW(snapshot, &PE))
	{
		do {
			if (wcscmp(PE.szExeFile, wname.c_str()) == 0) {
				CloseHandle(snapshot);
				return PE.th32ProcessID;
			}
		} while (Process32NextW(snapshot, &PE));
	}
	CloseHandle(snapshot);
	return 0;
}

DWORD Process::get_pid() { return this->pid; }

HANDLE Process::hijack_handle(DWORD pid)
{
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	typedef NTSTATUS(NTAPI* _RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
	auto RtlAdjustPrivilege = (_RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
	auto myNtQuery = (pNtQuerySystemInformation)GetProcAddress(ntdll, "NtQuerySystemInformation");

	BOOLEAN oldPriv;
	RtlAdjustPrivilege(20, TRUE, FALSE, &oldPriv);

	NTSTATUS status = 0;
	ULONG size = 0x10000;
	PSYSTEM_HANDLE_INFORMATION hInfo = (PSYSTEM_HANDLE_INFORMATION)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	while ((status = myNtQuery(SystemHandleInformation, hInfo, size, &size)) == 0xC0000004)
	{
		VirtualFree(hInfo, 0, MEM_RELEASE);
		size *= 2;
		hInfo = (PSYSTEM_HANDLE_INFORMATION)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	}

	if (!NT_SUCCESS(status) || !hInfo) {
		if (hInfo) VirtualFree(hInfo, 0, MEM_RELEASE);
		return nullptr;
	}

	HANDLE result = nullptr;
	OBJECT_ATTRIBUTES objAttr = { sizeof(OBJECT_ATTRIBUTES) };

	for (ULONG i = 0; i < hInfo->HandleCount; i++)
	{
		SYSTEM_HANDLE sh = hInfo->Handles[i];
		if (!sh.Handle) continue;

		CLIENT_ID CID = { (HANDLE)(ULONG_PTR)sh.ProcessId, nullptr };
		HANDLE ownerHandle = nullptr;

		// Use the member pointer initialized in constructor
		status = _NtOpenProcess(&ownerHandle, PROCESS_DUP_HANDLE, &objAttr, &CID);
		if (!NT_SUCCESS(status) || !ownerHandle) continue;

		HANDLE dupHandle = nullptr;
		// Ensure _NtDuplicateObject is added as a class member!
		status = _NtDuplicateObject(ownerHandle, (HANDLE)(ULONG_PTR)sh.Handle, GetCurrentProcess(), &dupHandle, PROCESS_ALL_ACCESS, 0, 0);
		_NtClose(ownerHandle);

		if (NT_SUCCESS(status) && dupHandle && GetProcessId(dupHandle) == pid)
		{
			result = dupHandle;
			break;
		}
		if (dupHandle) _NtClose(dupHandle);
	}

	VirtualFree(hInfo, 0, MEM_RELEASE);
	return result;
}

Process::Process(std::string name)
{
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	_NtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(ntdll, "NtReadVirtualMemory");
	_NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(ntdll, "NtWriteVirtualMemory");
	_NtProtectVirtualMemory = (pNtProtectVirtualMemory)GetProcAddress(ntdll, "NtProtectVirtualMemory");
	_NtAllocateVirtualMemory = (pNtAllocateVirtualMemory)GetProcAddress(ntdll, "NtAllocateVirtualMemory");
	_NtOpenProcess = (pNtOpenProcess)GetProcAddress(ntdll, "NtOpenProcess");
	_NtClose = (pNtClose)GetProcAddress(ntdll, "NtClose");
	_NtDuplicateObject = (pNtDuplicateObject)GetProcAddress(ntdll, "NtDuplicateObject");

	this->name = name;
	this->pid = GetPID(name);
	this->handle = hijack_handle(this->pid);

	if (!this->handle) std::cerr << "failed to hijack handle for: " << name << std::endl;
}

Process::~Process()
{
	if (handle) _NtClose(handle);
}

NTSTATUS Process::read_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T read) { return _NtReadVirtualMemory(this->handle, base, buf, size, read); }
NTSTATUS Process::write_mem(PVOID base, PVOID buf, SIZE_T size, PSIZE_T written) { return _NtWriteVirtualMemory(this->handle, base, buf, size, written); }
NTSTATUS Process::change_protection(PVOID base, SIZE_T size, ULONG prot, PULONG oldProt) { return _NtProtectVirtualMemory(this->handle, &base, &size, prot, oldProt); }
PVOID Process::allocate_mem(SIZE_T size, ULONG type, ULONG prot)
{
	PVOID base = nullptr;
	_NtAllocateVirtualMemory(this->handle, &base, 0, &size, type, prot);
	return base;
}