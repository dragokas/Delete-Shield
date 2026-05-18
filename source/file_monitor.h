#pragma once

#include "file_shield.h"
#include "wildcard_matcher.h"

#include <string>
#include <unordered_set>
#include <vector>
#include <atomic>
#include <thread>

class FileMonitor
{
public:
    FileMonitor(const std::wstring& rootDir) : _rootDir(rootDir) {};
    ~FileMonitor();
    
    void SetLockerState(bool value);
    void SetWildcardPatterns(const std::vector<std::wstring>& patterns);
    void Start();
    void Stop();
    
    static BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);

private:
    bool ShouldProcess(const std::wstring& filename) const;
    void PostRead();
    void IoCompletionThread();
    void ParseNotifications(BYTE* buffer, DWORD bytesTransferred);
    
    static constexpr DWORD _kBufferSize = 64 * 1024;
    static constexpr size_t _kOutstandingRequests = 4;

    struct IoContext
    {
        OVERLAPPED overlapped{};
        BYTE buffer[_kBufferSize] = {};
    };

    const std::wstring _rootDir;
    FileShield _FileShield;
    bool _isLockerEnabled = false;
    std::atomic<bool> _isRunning = false;
    HANDLE _hDir = INVALID_HANDLE_VALUE;
    HANDLE _hIocp = nullptr;
    std::thread _iocpThread;

    std::unordered_set<std::wstring> _createdFiles;
    WildcardMatcher _wildcardMatcher;
    
    static FileMonitor* _currentInstance;
};