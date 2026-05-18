#pragma once

#include <windows.h>
#include <unordered_map>
#include <string>
#include <filesystem>

class FileShield
{
public:
    FileShield() = default;
    ~FileShield() {
        if (HasLockedObjects())
        {
            UnlockAll();
        }
    }

    bool ProtectFile(const std::wstring& path);
    void UnlockAll();
    bool HasLockedObjects();
    static bool UnlockObject(const std::wstring& path);
    static bool UnlockDirectory(std::filesystem::path rootDir, bool recursive);

private:
    struct LockedFile
    {
        HANDLE handle = INVALID_HANDLE_VALUE;
        PSECURITY_DESCRIPTOR originalSD = nullptr;

        ~LockedFile()
        {
            if (handle != INVALID_HANDLE_VALUE) {
                CloseHandle(handle);
                handle = INVALID_HANDLE_VALUE;
            }
            if (originalSD) {
                LocalFree(originalSD);
                originalSD = nullptr;
            }
        }

        LockedFile() = default;

        // Block copy-semantics other than std::move
        LockedFile(const LockedFile&) = delete;
        LockedFile& operator=(const LockedFile&) = delete;
        LockedFile(LockedFile&& other) noexcept
            : handle(other.handle), originalSD(other.originalSD)
        {
            other.handle = INVALID_HANDLE_VALUE;
            other.originalSD = nullptr;
        }

        LockedFile& operator=(LockedFile&& other) noexcept
        {
            if (this != &other)
            {
                // Clean up current resources
                if (handle != INVALID_HANDLE_VALUE) {
                    CloseHandle(handle);
                    handle = INVALID_HANDLE_VALUE;
                }
                if (originalSD) {
                    LocalFree(originalSD);
                    originalSD = nullptr;
                }

                // Move from other
                handle = other.handle;
                originalSD = other.originalSD;

                other.handle = INVALID_HANDLE_VALUE;
                other.originalSD = nullptr;
            }
            return *this;
        }
    };

    bool AddDenyAce(const std::wstring& path, DWORD accessMask, PSECURITY_DESCRIPTOR& originalSD);
    bool Unlock(const std::wstring& path, const LockedFile& lf);

    std::unordered_map<std::wstring, LockedFile> _lockedFiles;
    std::unordered_map<std::wstring, LockedFile> _lockedDirectories;
};