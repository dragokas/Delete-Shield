#include "file_monitor.h"
#include "file_info.h"

FileMonitor* FileMonitor::_currentInstance = nullptr;

FileMonitor::~FileMonitor()
{
    SetConsoleCtrlHandler(nullptr, FALSE);
    _currentInstance = nullptr;
}

BOOL WINAPI FileMonitor::ConsoleHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT)
    {
        if (_currentInstance)
        {
            wprintf(L"\n\n[!] Ctrl+C detected, cleaning up...\n");
            _currentInstance->Stop();
            Sleep(100);
            ExitProcess(0);
        }
        return TRUE;
    }
    return FALSE;
}

void FileMonitor::SetLockerState(bool value)
{
    _isLockerEnabled = value;
}

void FileMonitor::SetWildcardPatterns(const std::vector<std::wstring>& patterns)
{
    _wildcardMatcher.AddPatterns(patterns);
}

bool FileMonitor::ShouldProcess(const std::wstring& fullPath) const
{
    if (!_wildcardMatcher.HasPatterns())
        return true;
    
    std::filesystem::path p(fullPath);
    std::wstring basename = p.filename().wstring();
    return _wildcardMatcher.Matches(basename);
}

void FileMonitor::Stop()
{
    if (!_isRunning)
        return;

    _isRunning = false;

    if (_hDir != INVALID_HANDLE_VALUE)
    {
        CancelIoEx(_hDir, nullptr);
    }

    // Wake IOCP thread
    //
    if (_hIocp)
    {
        PostQueuedCompletionStatus(_hIocp, 0, 0, nullptr);
    }

    if (_iocpThread.joinable())
    {
        _iocpThread.join();
    }

    if (_hIocp)
    {
        CloseHandle(_hIocp);
        _hIocp = nullptr;
    }

    if (_hDir != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_hDir);
        _hDir = INVALID_HANDLE_VALUE;
    }

    if (_currentInstance)
    {
        _FileShield.UnlockAll();
        _createdFiles.clear();

        SetConsoleCtrlHandler(nullptr, FALSE);
        _currentInstance = nullptr;
    }
}

void FileMonitor::Start()
{
    if (_isRunning)
        return;

    _currentInstance = this;
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    if (!std::filesystem::exists(_rootDir))
    {
        wprintf(L"Directory %s not exists!\n", _rootDir.c_str());
        return;
    }

    _hDir = CreateFileW(
        _rootDir.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        nullptr);

    if (_hDir == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Cannot open directory %s -> Error: %lu\n", _rootDir.c_str(), GetLastError());
        return;
    }

    _hIocp = CreateIoCompletionPort(_hDir, nullptr, 0, 0);
    if (!_hIocp)
    {
        wprintf(L"CreateIoCompletionPort failed -> Error: %lu\n", GetLastError());
        CloseHandle(_hDir);
        _hDir = INVALID_HANDLE_VALUE;
        return;
    }

    if (_wildcardMatcher.HasPatterns())
    {
        wprintf(L"Filtering by wildcards (case-insensitive)\n");
    }

    if (_isLockerEnabled)
    {
        wprintf(L"Blocking: %s\n", _rootDir.c_str());
    }
    else {
        wprintf(L"Monitoring: %s\n", _rootDir.c_str());
    }

    wprintf(L"Press ESC or Ctrl + C to exit.\n\n");

    _isRunning = true;

    // Keep multiple outstanding requests
    // to reduce event loss under heavy load
    //
    for (size_t i = 0; i < _kOutstandingRequests; ++i)
    {
        PostRead();
    }

    _iocpThread = std::thread(
        &FileMonitor::IoCompletionThread, this);
}

void FileMonitor::PostRead()
{
    IoContext* context = new IoContext();

    DWORD notifyFlags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

    if (_isLockerEnabled)
    {
        notifyFlags += FILE_NOTIFY_CHANGE_SIZE;
    }

    BOOL success = ReadDirectoryChangesW(
        _hDir,
        context->buffer,
        sizeof(context->buffer),
        TRUE,
        notifyFlags,
        nullptr,
        &context->overlapped,
        nullptr);

    if (!success)
    {
        wprintf(L"ReadDirectoryChangesW failed -> Error: %lu\n", GetLastError());
        delete context;
        return;
    }
}

void FileMonitor::IoCompletionThread()
{
    while (true)
    {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        LPOVERLAPPED overlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(
            _hIocp,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            INFINITE);

        if (!overlapped)
        {
            if (!_isRunning)
                break;

            continue;
        }

        IoContext* context = CONTAINING_RECORD(overlapped, IoContext, overlapped);

        if (!result)
        {
            DWORD err = GetLastError();
            if (err != ERROR_OPERATION_ABORTED)
            {
                wprintf(L"GetQueuedCompletionStatus failed -> Error: %lu\n", err);
            }
            delete context;
            continue;
        }

        if (bytesTransferred > 0)
        {
            ParseNotifications(context->buffer, bytesTransferred);
        }

        delete context;

        if (_isRunning.load())
        {
            PostRead();
        }
    }
}

void FileMonitor::ParseNotifications(BYTE* buffer, DWORD bytesTransferred)
{
    BYTE* ptr = buffer;

    while (true)
    {
        auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr);

        std::wstring name(info->FileName, info->FileNameLength / sizeof(WCHAR));
        std::wstring fullPath = _rootDir + L"\\" + name;
        
        if (!ShouldProcess(fullPath))
        {
            goto next_entry;
        }

        if (info->Action == FILE_ACTION_ADDED)
        {
            if (_isLockerEnabled)
            {
                if (FileInfo::IsDirectory(fullPath))
                {
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
                    _createdFiles.insert(fullPath);
                }
            }
            else {
				if (FileInfo::IsDirectory(fullPath))
				{
					wprintf(L"[CREATED DIR] %s\n", fullPath.c_str());
				}
				else {
					wprintf(L"[CREATED FILE] %s\n", fullPath.c_str());
				}
            }
        }
        else if (info->Action == FILE_ACTION_MODIFIED)
        {
            // Checking is it newly created file
            if (_createdFiles.contains(fullPath))
            {
                // Remove from the cache, as we no longer need to receive the event for this file
                _createdFiles.erase(fullPath);
                _FileShield.ProtectFile(fullPath);
            }
        }

next_entry:
        if (info->NextEntryOffset == 0)
        {
            break;
        }
        ptr += info->NextEntryOffset;
    }
}