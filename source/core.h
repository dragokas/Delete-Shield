#pragma once

#include <string>
#include <vector>

class Core
{
	Core() = delete;

public:
	static void StartMonitor(const std::wstring& rootDir, 
	                        const std::vector<std::wstring>& wildcards = {});
	static void StartBlocker(const std::wstring& rootDir, 
	                         const std::vector<std::wstring>& wildcards = {});
	static void StartUnloker(const std::wstring& rootDir);
	static void SetupPrivileges();
};