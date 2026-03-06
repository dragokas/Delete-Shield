#include "path.h"

#include <windows.h>

std::filesystem::path Path::NormalizePath(const std::wstring& path)
{
    std::wstring str(path);
    if (!str.empty() && str.front() == L'"') {
        str.erase(0, 1);
    }
    if (!str.empty() && str.back() == L'"') {
        str.pop_back();
    }
    if (!str.empty() && str.back() == L':') {
        str += L'\\';
    }
    std::filesystem::path normPath(str);
    normPath = std::filesystem::canonical(normPath);
    return normPath;
}

bool Path::NormalizePath(const std::wstring& path, std::filesystem::path& normalizedPath)
{
    std::wstring normPath;
    try
    {
        normalizedPath = Path::NormalizePath(path);
    }
    catch (const std::exception& e)
    {
        printf("Error: %s\n", e.what());
        return false;
    }
    return true;
}

std::wstring Path::GetExecutableFilename()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).filename().wstring();
}

std::filesystem::path Path::GetTempDirectoryPath()
{
    wchar_t buffer[MAX_PATH];
    GetTempPath(MAX_PATH, buffer);
    return NormalizePath(buffer);
}