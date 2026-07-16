#pragma once
#include <windows.h>

/* if you didnt notice, i took a lot inspiration from MinHook */


/* !! THIS IS UNSTABLE !! */

/* codes */
typedef enum _EH_STATUS
{
	EH_OK = 0,
	EH_PROCESS_NOT_FOUND,
	EH_HANDLE_NOT_FOUND,
	EH_INVALID_ADDRESS,
	EH_ALLOCATE_FAIL,
	EH_MAPPING_FAILED,
	EH_HOOK_FAILED,
	EH_ERROR_INTENRAL
} EH_STATUS;


#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
	#error "x86-x64 are the only supported types" 
#endif

#if defined(_MSVC_LANG)
	#define CURRENT_STD _MSVC_LANG
#else
	#define CURRENT_STD __cplusplus
#endif

#if CURRENT_STD < 201703L
	#error "This code requires C++17 or later. Please update your compiler or change your settings."
#endif

#ifdef __cplusplus
extern "C" {
#endif

	
	/* 
	* Initialize this library, call this only ONCE ever, at the beginning of your program.
	*
	* processName -> Name of the process you want to target.
	* Usage: if (EH_Initialize("RobloxPlayerBeta.exe") != EH_OK) { return; } 
	*/
	EH_STATUS __stdcall EH_Initialize(const char* processName);
	
	/* 
	* Uninitialize this library, call this only ONCE ever, at the end of your program.
	* Usage: (pretend we are at the end of the file) if(EH_Unitialize() != EH_OK) { return; }
	*/
	EH_STATUS __stdcall EH_Uninitialize(VOID);

	/*  
	* Maps your dll into memory into your target pid.
	* 
	* DLlPath -> The absoulte path to your DLL.
	* outEntryPoint -> The address of your entry point. (!!NEEDED FOR HOOKING!!)
	* Usage: if (EH_Map(C:\\Users\\Public\\mycooldll.dll) != EH_OK) { return; }
	*/
	EH_STATUS __stdcall EH_Map(const wchar_t* DllPath, uintptr_t* outEntryPoint);
	
	/* 
	* Creates an external hook via pointer to pointer hooking (IAT/VMT swapping).
	* 
	* jmpInstruction -> The function you want to hook on.
	* newTarget -> Your exported function in your dll (edit mmap.cpp for that).
	* 
	* The entry.remoteBase is from my mmaper. If you want to put your own
	* custom mmaper, just comment out EH_Map and remove mmap.h/mmap.cpp.
	* 
	* Usage: if (EH_CreateHook(QPC, entry.remoteBase) != EH_OK) { return; }
	* OR
	* uintptr_t entry = 0;
	* if (EH_Map("dllpath.dll", &entry) == EH_OK { EH_CreateHook(targetAddr, (void*)entry); }
	*/
	EH_STATUS __stdcall EH_CreateHook(void* jmpInstruction, void* newTarget);

	/* 
	* Call this function at the end of the file, ONCE, before EH_Unitialize
	* Or you can just call EH_Unititialize since it calls this function. Your choice.
	*/

	EH_STATUS __stdcall EH_CleanHook();


#ifdef __cplusplus
}
#endif