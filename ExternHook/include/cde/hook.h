#pragma once
#include "process.h"

struct Hook
{
	void** targetSlot; /* Address of the pointer we're replacing. */
	void* orig; /* Original function pointer. */

	/* Hook. */
	bool hook(Process& proc, void* jmpInstruction, void* newTarget)
	{
		/* 
		* Calculate the address of the pointer slot. 
		* jmp [rip + offset] - offset is at byte 3. Pointer is at instruction + 7 + offset.
		*/
		int32_t offset = 0;
		SIZE_T read;
		proc.read_mem((PVOID)((uintptr_t)jmpInstruction + 3), &offset, sizeof(int32_t), &read);
		targetSlot = (void**)((uintptr_t)jmpInstruction + 7 + offset);

		/* Read original memory. */
		proc.read_mem(targetSlot, &orig, sizeof(void*), &read);

		/* Write the new target. */
		ULONG oldProt;
		proc.change_protection(targetSlot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
		SIZE_T written;
		proc.write_mem(targetSlot, &newTarget, sizeof(void*), &written);
		proc.change_protection(targetSlot, sizeof(void*), oldProt, &oldProt);

		/* We are hooked! */
		return true;
	}

	/* Unhook. */
	bool unhook(Process& proc)
	{
		ULONG oldProt;
		proc.change_protection(targetSlot, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProt);
		SIZE_T written;
		proc.write_mem(targetSlot, &orig, sizeof(void*), &written);
		proc.change_protection(targetSlot, sizeof(void*), oldProt, &oldProt);
		/* We are unhooked! */
		return true;
	}
};