#include <windows.h>


/* Simple function that just logs a file. */
extern "C" __declspec(dllexport) void ThreadpoolCallback()
{
    static bool already_ran = false;
    if (already_ran) return;
    already_ran = true;

    HANDLE hFile = CreateFileA("C:\\Users\\Public\\log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        const char* text = "DLL executed successfully!";
        DWORD bytesWritten = 0;
        WriteFile(hFile, text, 33, &bytesWritten, NULL);
        CloseHandle(hFile);
    }

}
