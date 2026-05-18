#pragma once

#include "file_shield.h"
#include "wildcard_matcher.h"

#include <string>
#include <unordered_set>
#include <vector>

class FileMonitor
{
public:
    FileMonitor(const std::wstring& rootDir) : _rootDir(rootDir) {};
    ~FileMonitor();
    
    void SetLockerState(bool value);
    void SetWildcardPatterns(const std::vector<std::wstring>& patterns);
    void Start();
    
    static BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);

private:
    bool ShouldProcess(const std::wstring& filename) const;
    void Cleanup();
    
    const std::wstring _rootDir;
    FileShield _FileShield;
    bool _isLockerEnabled = false;
    bool _isRunning = true;
    
    std::unordered_set<std::wstring> _createdFiles;
    WildcardMatcher _wildcardMatcher;
    
    static FileMonitor* _currentInstance;
};