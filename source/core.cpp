#include "core.h"
#include "file_monitor.h"
#include "file_shield.h"
#include "process_priority.h"
#include "process_token.h"

void Core::StartMonitor(const std::wstring& rootDir)
{
    FileMonitor monitor(rootDir);
    monitor.SetLockerState(false);
    monitor.Start();
}

void Core::StartBlocker(const std::wstring& rootDir)
{
    FileMonitor monitor(rootDir);
    monitor.SetLockerState(true);
    monitor.Start();
}

void Core::StartUnloker(const std::wstring& rootDir)
{
    FileShield::UnlockDirectory(rootDir, true);
}

void Core::SetupPrivileges()
{
    ProcessPriority::SetPriority(ProcessPriority::ProcessPriorityLevel::High);
    ProcessToken::EnablePrivilege(L"SeBackupPrivilege");        // bypass DACL read
    ProcessToken::EnablePrivilege(L"SeRestorePrivilege");       // bypass DACL write
    ProcessToken::EnablePrivilege(L"SeTakeOwnershipPrivilege");
    ProcessToken::EnablePrivilege(L"SeDebugPrivilege");         // always useful
}