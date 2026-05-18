#pragma once

class Console
{
	Console() = delete;

public:
	static void SetIcon();
	static void SetTitle();
	static void SetUnicode();
	static void WaitKey(int virtualCode);
};

