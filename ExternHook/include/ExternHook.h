#pragma once
#include <windows.h>

/* if you didnt notice, i took a lot inspiration from MinHook */



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

typedef void* EH_HOOK;

#define EH_DESTROY nullptr;


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
	* IN - processName -> Name of the process you want to target.
	* Usage: if (EH_Initialize("RobloxPlayerBeta.exe") != EH_OK) { return; } 
	*/
	EH_STATUS __stdcall EH_Initialize(const char* processName);
	
	/* 
	* Uninitialize this library, call this only ONCE ever, at the end of your program.
	* Usage: (pretend we are at the end of the file) if (EH_Unitialize() != EH_OK) { return; }
	*/
	EH_STATUS __stdcall EH_Uninitialize(VOID);

	/*  
	* Maps your dll into memory into your target pid.
	* 
	* IN - DLlPath -> The absoulte path to your DLL.
	* OUT - outEntryPoint -> The address of your entry point. (!!NEEDED FOR HOOKING!!)
	* Usage: if (EH_Map(C:\\Users\\Public\\mycooldll.dll) != EH_OK) { return; }
	*/
	EH_STATUS __stdcall EH_Map(const wchar_t* DllPath, uintptr_t* outEntryPoint);
	
	/* 
	* Creates an external hook via IAT hooking.
	* 
	* IN - jmpInstruction -> The function you want to hook on.
	* IN - newTarget -> Your exported function in your dll (edit mmap.cpp for that).
	* OUT - outHook -> Stores the hook pointer, to destroy later with EH_CleanupHook.
	* 
	* 
	* The entry.remoteBase is from my mmaper. If you want to put your own
	* custom mmaper, just comment out EH_Map and remove mmap.h/mmap.cpp.
	* 
	* Usage: if (EH_CreateHook(QPC, entry.remoteBase) != EH_OK) { return; }
	* OR
	* uintptr_t entry = 0;
	* if (EH_Map("dllpath.dll", &entry) == EH_OK { EH_CreateHook(targetAddr, (void*)entry); }
	*/
	EH_STATUS __stdcall EH_CreateHook(void* jmpInstruction, void* newTarget, EH_HOOK* outHook);

	/* 
	* Call this function to clean up a specific hook.
	* If you want to destroy every hook, use EH_DESTROY 
	* 
	* IN - hook -> Cleans only one specific hook.
	* 
	* Usage: EH_STATUS status = EH_CleanHook(qpcHook);
	* OR
	* EH_STATUS status = EH_CleanHook(EH_DESTROY);
	*/
	EH_STATUS __stdcall EH_CleanHook(EH_HOOK hook);


#ifdef __cplusplus
}
#endif
