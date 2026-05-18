#include "core.h"
#include "file_monitor.h"
#include "file_shield.h"
#include "process_priority.h"
#include "process_token.h"
#include "console.h"

void Core::StartMonitor(const std::wstring& rootDir, 
                        const std::vector<std::wstring>& wildcards)
{
    FileMonitor monitor(rootDir);
    monitor.SetWildcardPatterns(wildcards);
    monitor.SetLockerState(false);
    monitor.Start();
    Console::WaitKey(VK_ESCAPE);
    monitor.Stop();
}

void Core::StartBlocker(const std::wstring& rootDir, 
                        const std::vector<std::wstring>& wildcards)
{
    FileMonitor monitor(rootDir);
    monitor.SetWildcardPatterns(wildcards);
    monitor.SetLockerState(true);
    monitor.Start();
    Console::WaitKey(VK_ESCAPE);
    monitor.Stop();
}

bool Core::StartUnloker(const std::wstring& rootDir)
{
    wprintf(L"Unlocking directory: %s\n", rootDir.c_str());
    wprintf(L"Note: only locked objects will be handled.\n");
    return FileShield::UnlockDirectory(rootDir, true);
}

void Core::SetupPrivileges()
{
    ProcessPriority::SetPriority(ProcessPriority::ProcessPriorityLevel::High);
    ProcessToken::EnablePrivilege(L"SeBackupPrivilege");        // bypass DACL read
    ProcessToken::EnablePrivilege(L"SeRestorePrivilege");       // bypass DACL write
    ProcessToken::EnablePrivilege(L"SeTakeOwnershipPrivilege");
    ProcessToken::EnablePrivilege(L"SeDebugPrivilege");         // always useful
}