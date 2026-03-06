#include "process_priority.h"

bool ProcessPriority::SetPriority(ProcessPriorityLevel level)
{
	HANDLE hProcess = GetCurrentProcess();
	return SetPriorityClass(hProcess, level);
}