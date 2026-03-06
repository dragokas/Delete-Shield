#pragma once

#include <string>

class Core
{
	Core() = delete;

public:
	static void StartMonitor(const std::wstring& rootDir);
	static void StartBlocker(const std::wstring& rootDir);
	static void StartUnloker(const std::wstring& rootDir);
	static void SetupPrivileges();
};