#pragma once

#include <windows.h>
#include <unordered_map>
#include <string>
#include <filesystem>

class FileShield
{
public:
    FileShield() = default;
    bool ProtectFile(const std::wstring& path);
    void UnlockAll(); // Unlocks previously protected files in this session only (not used)
    static bool UnlockObject(const std::wstring& path);
    static bool UnlockDirectory(std::filesystem::path rootDir, bool recursive);

private:
    struct LockedFile
    {
        HANDLE handle = INVALID_HANDLE_VALUE;
        PACL originalDacl = nullptr;
    };

    bool AddDenyAce(const std::wstring& path, DWORD accessMask, PACL& originalDacl);
    bool Unlock(const std::wstring& path, const LockedFile& lf);

    std::unordered_map<std::wstring, LockedFile> _lockedFiles;
    std::unordered_map<std::wstring, LockedFile> _lockedDirectories;
};
