#pragma once

#include "yield.h"
#include <filesystem>

class FileEnumerator
{
	FileEnumerator() = delete;

public:
	static Generator<std::filesystem::path> Search(std::filesystem::path rootDir, bool recursive);
};