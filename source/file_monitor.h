#pragma once

#include "file_shield.h"

#include <string>

class FileMonitor
{
public:
	FileMonitor(const std::wstring rootDir) : _rootDir(rootDir) {};
	void SetLockerState(bool value);
	void Start();

private:
	const std::wstring _rootDir;
	FileShield _FileShield;
	bool _isLockerEnabled = false;
};