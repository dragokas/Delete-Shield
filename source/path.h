#pragma once

#include <string>
#include <filesystem>

class Path
{
	Path() = delete;

public:
	static std::filesystem::path NormalizePath(const std::wstring& path);
	static bool NormalizePath(const std::wstring& path, std::filesystem::path& normalizedPath);
	static std::wstring GetExecutableFilename();
	static std::filesystem::path GetTempDirectoryPath();
};

