#ifndef COMMON_H
#define COMMON_H


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <chrono>
#include <string>
#include <iostream>

#define PRINT_LAST_ERROR(msg) PrintLastError(GetLastError(), msg)
#define PRINT_WSA_LAST_ERROR(msg) PrintLastError(WSAGetLastError(), msg)
#define PRINT_LAST_ERROR_DEFAULT() PrintLastError(GetLastError(), L"WinApi error")
#define PRINT_WSA_LAST_ERROR_DEFAULT() PrintLastError(WSAGetLastError(), L"WinSock error")

long long GetHighResolutionTimeMS();

long long GetHighResolutionTimeNS();

void PrintLastError(DWORD error_code, const std::wstring& msg = L"");

#endif // COMMON_H