#pragma once

#include <vector>
#include <string>

class CommandLine
{
	CommandLine() = delete;

public:
	struct ParsedArgs
	{
		wchar_t mode = 0;
		std::wstring path;
		std::vector<std::wstring> wildcards;
	};
	
	static int Parse(int argc, wchar_t* argv[]);
	static void ShowAppName();
	static void ShowUsage();
};