#pragma once
#include <windows.h>

#ifdef _DEBUG
#include <stdio.h>

#define dbgstart()                                     \
    do                                                 \
    {                                                  \
        AllocConsole();                                \
        FILE* fp;                                      \
        freopen_s(&fp, "CON", "r", stdin);             \
        freopen_s(&fp, "CON", "w", stdout);            \
        freopen_s(&fp, "CON", "w", stderr);            \
    } while (0)

#define dbgend()            FreeConsole()
#define dbg(...)            printf(__VA_ARGS__)

#else

#define dbgstart()          ((void)0)
#define dbgend()            FALSE
#define dbg(...)            (-1)

#endif

// Return win32 error code
EXTERN_C DWORD PrepareForUIAccess();