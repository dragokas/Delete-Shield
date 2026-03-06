#include "file_monitor.h"
#include "file_info.h"

#include <unordered_set>

std::unordered_set<std::wstring> _createdFiles;

void FileMonitor::SetLockerState(bool value)
{
    _isLockerEnabled = value;
}

void FileMonitor::Start()
{
    if (!std::filesystem::exists(_rootDir))
    {
        wprintf(L"Directory %s not exists!\n", _rootDir.c_str());
        return;
    }

    HANDLE hDir = CreateFileW(
        _rootDir.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (hDir == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Cannot open directory %s -> Error: %lu\n", _rootDir.c_str(), GetLastError());
        return;
    }

    if (_isLockerEnabled)
    {
        wprintf(L"Blocking: %s\n", _rootDir.c_str());
    }
    else {
        wprintf(L"Monitoring: %s\n", _rootDir.c_str());
    }

    wprintf(L"Press Ctrl + C to exit.\n\n");

    BYTE buffer[8192];
    DWORD bytesReturned;

    DWORD notifyFlags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

    if (_isLockerEnabled)
    {
        notifyFlags += FILE_NOTIFY_CHANGE_SIZE;
    }

    while (true)
    {
        if (ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            TRUE,
            notifyFlags,
            &bytesReturned,
            nullptr,
            nullptr))
        {
            FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)buffer;
            do
            {
                std::wstring name(info->FileName, info->FileNameLength / sizeof(WCHAR));
                std::wstring fullPath = _rootDir + L"\\" + name;

                if (info->Action == FILE_ACTION_ADDED)
                {
                    if (_isLockerEnabled)
                    {
                        if (FileInfo::IsDirectory(fullPath))
                        {
                            //wprintf(L"[CREATED DIR] %s\n", fullPath.c_str());
                            _FileShield.ProtectFile(fullPath);
                        }
                        else {
                            // Some installers tend to create empty file to check whether the location writable.
                            // Then, they instantly removes this file.
                            // If DELETE operation locked, the installer is no longer able to create directory of the same name.
                            // With _createdFiles cache, the DELETE lock is postponed until we receive
                            // the FILE_NOTIFY_CHANGE_SIZE notification.
                            // * Also, sometimes GetFileSizeEx reports non-zero size of the above dummy file,
                            // so, the check (size != 0) is not appropriate here
                            //wprintf(L"[CREATED FILE] %s, size = %llu\n", fullPath.c_str(), FileInfo::GetSize(fullPath));
                            _createdFiles.insert(fullPath);
                        }
                    }
                    else {
                        wprintf(L"[CREATED] %s\n", fullPath.c_str());
                    }
                }
                else if (info->Action == FILE_ACTION_MODIFIED)
                {
                    // Checking is it newly created file
                    if (_createdFiles.contains(fullPath))
                    {
                        // Remove from the cache, as we no longer need to receive the event for this file
                        _createdFiles.erase(fullPath);
                        //wprintf(L"[CHANGE SIZE] %s, size = %llu\n", fullPath.c_str(), FileInfo::GetSize(fullPath));
                        _FileShield.ProtectFile(fullPath);
                    }
                }

                if (info->NextEntryOffset == 0)
                {
                    break;
                }
                info = (FILE_NOTIFY_INFORMATION*)((BYTE*)info + info->NextEntryOffset);
            } while (true);
        }
    }

    CloseHandle(hDir);
}
