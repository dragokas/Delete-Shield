#pragma once

#include <windows.h>

class ProcessPriority
{
	ProcessPriority() = delete;

public:
	enum ProcessPriorityLevel : DWORD
	{	
		BackgroundBegin = PROCESS_MODE_BACKGROUND_BEGIN,
		BackgroundEnd = PROCESS_MODE_BACKGROUND_END,
		Idle = IDLE_PRIORITY_CLASS,
		BelowNormal = BELOW_NORMAL_PRIORITY_CLASS,
		Normal = NORMAL_PRIORITY_CLASS,
		AboveNormal = ABOVE_NORMAL_PRIORITY_CLASS,
		High = HIGH_PRIORITY_CLASS,
		Realtime = REALTIME_PRIORITY_CLASS
	};

	static bool SetPriority(ProcessPriorityLevel level);
};