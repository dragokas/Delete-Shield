#pragma once

#include <windows.h>
#include <string>

class ProcessToken
{
	ProcessToken() = delete;

public:
	static bool EnablePrivilege(const std::wstring& privilege);
};