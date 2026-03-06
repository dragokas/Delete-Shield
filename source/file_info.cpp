#include "file_info.h"

LONGLONG FileInfo::GetSize(const std::wstring& path)
{
    LARGE_INTEGER size;

    HANDLE h = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (h != INVALID_HANDLE_VALUE)
    {
        GetFileSizeEx(h, &size);
        CloseHandle(h);
    }
    return size.QuadPart;
}

DWORD FileInfo::GetAttributes(const std::wstring& path)
{
    return GetFileAttributes(path.c_str());
}

bool FileInfo::IsDirectory(const std::wstring& path)
{
    return ((GetAttributes(path) & FILE_ATTRIBUTE_DIRECTORY) != 0);
}