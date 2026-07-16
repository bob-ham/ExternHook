#include <windows.h>

#include <windows.h>

extern "C" __declspec(dllexport) void ThreadpoolCallback()
{
    static bool already_ran = false;
    if (already_ran) return;
    already_ran = true;

    // Create a file named log.txt
    HANDLE hFile = CreateFileA(
        "C:\\Users\\Public\\log.txt",                  // Filename
        GENERIC_WRITE,              // Desired access
        FILE_SHARE_READ,            // Share mode
        NULL,                       // Security attributes
        CREATE_ALWAYS,              // Disposition (always create new)
        FILE_ATTRIBUTE_NORMAL,      // File attributes
        NULL                        // Template file
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        const char* text = "Callback executed successfully!";
        DWORD bytesWritten = 0;

        // Write a small message to the file
        WriteFile(hFile, text, 33, &bytesWritten, NULL);

        // Close the file handle
        CloseHandle(hFile);
    }
}