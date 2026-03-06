#pragma once

class CommandLine
{
	CommandLine() = delete;

public:
	static int Parse(int argc, wchar_t* argv[]);
	static void ShowAppName();
	static void ShowUsage();
};