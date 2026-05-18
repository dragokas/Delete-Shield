#include "file_shield.h"
#include "file_enumerator.h"

#include <aclapi.h>
#include <sddl.h>
#include <string>
#include <filesystem>

#pragma comment(lib, "Advapi32.lib")

bool FileShield::AddDenyAce(const std::wstring& path, DWORD accessMask, PACL& originalDacl)
{
    PACL oldDacl = nullptr;
    PSECURITY_DESCRIPTOR sd = nullptr;

    DWORD res = GetNamedSecurityInfoW(
        path.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr, nullptr,
        &oldDacl,
        nullptr,
        &sd);

    if (res != ERROR_SUCCESS) {
        wprintf(L"GetNamedSecurityInfoW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        return false;
    }

    originalDacl = oldDacl;

    PSID pSid = nullptr;
    if (!ConvertStringSidToSidW(L"S-1-1-0", &pSid)) { // Everyone
        wprintf(L"ConvertStringSidToSidW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        return false;
    }

    EXPLICIT_ACCESSW ea = {};
    ea.grfAccessPermissions = accessMask;
    ea.grfAccessMode = DENY_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;

    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPWSTR)pSid;

    PACL newDacl = nullptr;

    res = SetEntriesInAclW(1, &ea, oldDacl, &newDacl);
    if (res != ERROR_SUCCESS) {
        wprintf(L"SetEntriesInAclW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        return false;
    }

    res = SetNamedSecurityInfoW(
        (LPWSTR)path.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr, nullptr,
        newDacl,
        nullptr);

    if (res != ERROR_SUCCESS)
    {
        wprintf(L"SetNamedSecurityInfoW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
    }

    LocalFree(newDacl);
    LocalFree(sd); // Important: free security descriptor!

    return res == ERROR_SUCCESS;
}

std::wstring GetParentDirectory(const std::wstring& path)
{
    std::filesystem::path p(path);
    return p.parent_path().wstring();
}

bool FileShield::ProtectFile(const std::wstring& path)
{
    bool result = false;

    if (_lockedFiles.count(path))
        return true;

    bool isDirectory = std::filesystem::is_directory(path);

    HANDLE h = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    // Method 1. Lock with CreateFileW (prevent FILE_SHARE_DELETE)
    if (h != INVALID_HANDLE_VALUE)
    {
        LockedFile lf;
        lf.handle = h;
        _lockedFiles[path] = std::move(lf);  // FIXED: use std::move
        wprintf(isDirectory ? L"[HANDLE LOCK DIR] %s\n" : L"[HANDLE LOCK FILE] %s\n", path.c_str());
        result = true;
    }
    else {
        DWORD err = GetLastError();
        if (err != ERROR_SHARING_VIOLATION && err != ERROR_ACCESS_DENIED)
        {
            wprintf(L"CreateFileW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        }
    }

    // Method 2. Lock with deny ACE: DELETE + FILE_DELETE_CHILD
    if (!result)
    {
        LockedFile lf;
        if (AddDenyAce(path, DELETE, lf.originalDacl))
        {
            _lockedFiles[path] = std::move(lf);  // FIXED: use std::move
            wprintf(isDirectory ? L"[ACL LOCK DIR] %s\n" : L"[ACL LOCK FILE] %s\n", path.c_str());
            result = true;
        }
    }

    auto parentDir = GetParentDirectory(path);

    if (!_lockedDirectories.count(parentDir))
    {
        PACL dirDacl = nullptr;
        if (AddDenyAce(parentDir, FILE_DELETE_CHILD, dirDacl))
        {
            LockedFile lf;
            lf.originalDacl = dirDacl;
            _lockedDirectories[parentDir] = std::move(lf);  // FIXED: use std::move
            wprintf(L"[ACL LOCK DIR] %s\n", parentDir.c_str());
        }
    }
    return result;
}

// Rest of the file remains the same...
bool FileShield::UnlockDirectory(std::filesystem::path rootDir, bool recursive)
{
    bool ret = true;

    if (!std::filesystem::exists(rootDir))
    {
        wprintf(L"Directory %s not exists!\n", rootDir.c_str());
        return false;
    }

    ret = ret && UnlockObject(rootDir);

    if (std::filesystem::is_directory(rootDir))
    {
        for (auto file : FileEnumerator::Search(rootDir, recursive))
        {
            ret = ret && UnlockObject(file);
        }
    }
    return ret;
}

bool FileShield::UnlockObject(const std::wstring& path)
{
    bool ret = false;

    PACL pOldDACL = nullptr;
    PSECURITY_DESCRIPTOR pSD = nullptr;

    DWORD result = GetNamedSecurityInfoW(
        path.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        &pOldDACL,
        nullptr,
        &pSD);

    if (result != ERROR_SUCCESS) {
        wprintf(L"GetNamedSecurityInfoW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        return false;
    }

    ACL_SIZE_INFORMATION aclSizeInfo = {};
    if (!GetAclInformation(pOldDACL, &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
        wprintf(L"GetAclInformation() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        if (pSD) LocalFree(pSD);
        return false;
    }

    DWORD newAclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL);
    PACL pNewDACL = (PACL)LocalAlloc(LMEM_ZEROINIT, newAclSize);
    if (!pNewDACL) {
        wprintf(L"LocalAlloc() call failed for ACL size %lu bytes, Error: %lu\n", newAclSize, GetLastError());
        if (pSD) LocalFree(pSD);
        return false;
    }

    if (!InitializeAcl(pNewDACL, newAclSize, ACL_REVISION)) {
        wprintf(L"InitializeAcl() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        LocalFree(pNewDACL);
        if (pSD) LocalFree(pSD);
        return false;
    }

    // Re-create new DACL without "Deny" ACEs
    bool hasDenyAces = false;
    for (DWORD i = 0; i < aclSizeInfo.AceCount; ++i) {
        LPVOID pAce = nullptr;
        if (GetAce(pOldDACL, i, &pAce)) {
            ACE_HEADER* header = (ACE_HEADER*)pAce;
            if (header->AceType == ACCESS_DENIED_ACE_TYPE) {
                hasDenyAces = true;
            }
            else {
                if (!AddAce(pNewDACL, ACL_REVISION, MAXDWORD, pAce, header->AceSize))
                {
                    wprintf(L"AddAce() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
                    LocalFree(pNewDACL);
                    if (pSD) LocalFree(pSD);
                    return false;
                }
            }
        }
    }

    if (!hasDenyAces) // no restrictions found -> just exit
    {
        LocalFree(pNewDACL);
        if (pSD) LocalFree(pSD);
        return true;
    }

    result = SetNamedSecurityInfoW(
        (LPWSTR)path.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        pNewDACL,
        nullptr);

    if (result != ERROR_SUCCESS) {
        wprintf(L"SetNamedSecurityInfoW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
    }
    else {
        wprintf(L"[UNLOCK] %s\n", path.c_str());
        ret = true;
    }

    LocalFree(pNewDACL);
    if (pSD) LocalFree(pSD);

    return ret;
}

bool FileShield::Unlock(const std::wstring& path, const LockedFile& lf)
{
    bool result = false;

    if (lf.handle != INVALID_HANDLE_VALUE) {
        // Handle will be closed in LockedFile destructor
        result = true;
    }

    if (lf.originalDacl)
    {
        if (ERROR_SUCCESS == SetNamedSecurityInfoW(
            (LPWSTR)path.c_str(),
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION,
            nullptr, nullptr,
            lf.originalDacl,
            nullptr))
        {
            result = true;
        }
        else {
            wprintf(L"SetNamedSecurityInfoW() call failed for: %s -> Error: %lu\n", path.c_str(), GetLastError());
        }
    }
    return result;
}

void FileShield::UnlockAll()
{
    for (auto& [path, lf] : _lockedFiles)
    {
        Unlock(path, lf);
        wprintf(L"[UNLOCK] %s\n", path.c_str());
    }

    for (auto& [path, lf] : _lockedDirectories)
    {
        Unlock(path, lf);
        wprintf(L"[UNLOCK] %s\n", path.c_str());
    }

    _lockedFiles.clear();
    _lockedDirectories.clear();
}