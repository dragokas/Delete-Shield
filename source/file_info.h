#pragma once

#include <windows.h>
#include <string>

class FileInfo
{
	FileInfo() = delete;

public:
	static LONGLONG GetSize(const std::wstring& path);
	static DWORD GetAttributes(const std::wstring& path);
	static bool IsDirectory(const std::wstring& path);
};