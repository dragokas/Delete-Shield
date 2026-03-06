#include "process_token.h"

bool ProcessToken::EnablePrivilege(const std::wstring& privilege)
{
    HANDLE token = nullptr;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
    {
        wprintf(L"OpenProcessToken() call failed -> Error: %lu\n", GetLastError());
        return false;
    }

    TOKEN_PRIVILEGES tp {};
    LUID luid {};

    if (!LookupPrivilegeValueW(nullptr, privilege.c_str(), &luid))
    {
        CloseHandle(token);
        wprintf(L"LookupPrivilegeValueW() call failed -> Error: %lu\n", GetLastError());
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(token,
        FALSE,
        &tp,
        sizeof(tp),
        nullptr,
        nullptr) || GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        CloseHandle(token);
        wprintf(L"AdjustTokenPrivileges() call failed for %s -> Error: %lu\n", privilege.c_str(), GetLastError());
        return false;
    }
    
    CloseHandle(token);
    return true;
}