#pragma once

#include <windows.h>
#include <unordered_map>
#include <string>
#include <filesystem>

class FileShield
{
public:
    FileShield() = default;
    ~FileShield() { UnlockAll(); }

    bool ProtectFile(const std::wstring& path);
    void UnlockAll();
    static bool UnlockObject(const std::wstring& path);
    static bool UnlockDirectory(std::filesystem::path rootDir, bool recursive);

private:
    struct LockedFile
    {
        HANDLE handle = INVALID_HANDLE_VALUE;
        PACL originalDacl = nullptr;

        ~LockedFile()
        {
            if (handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
            if (originalDacl)
                LocalFree(originalDacl);
        }

        LockedFile() = default;
        LockedFile(const LockedFile&) = delete;
        LockedFile& operator=(const LockedFile&) = delete;
        LockedFile(LockedFile&& other) noexcept
            : handle(other.handle), originalDacl(other.originalDacl)
        {
            other.handle = INVALID_HANDLE_VALUE;
            other.originalDacl = nullptr;
        }

        LockedFile& operator=(LockedFile&& other) noexcept
        {
            if (this != &other)
            {
                // Clean up current resources
                if (handle != INVALID_HANDLE_VALUE)
                    CloseHandle(handle);
                if (originalDacl)
                    LocalFree(originalDacl);

                // Move from other
                handle = other.handle;
                originalDacl = other.originalDacl;

                other.handle = INVALID_HANDLE_VALUE;
                other.originalDacl = nullptr;
            }
            return *this;
        }
    };

    bool AddDenyAce(const std::wstring& path, DWORD accessMask, PACL& originalDacl);
    bool Unlock(const std::wstring& path, const LockedFile& lf);

    std::unordered_map<std::wstring, LockedFile> _lockedFiles;
    std::unordered_map<std::wstring, LockedFile> _lockedDirectories;
};