#pragma once

#include <string>

#define BLACK(text)        (std::wstring(1, 0xE000 + 0) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define BLUE(text)         (std::wstring(1, 0xE000 + 1) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define GREEN(text)        (std::wstring(1, 0xE000 + 2) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define CYAN(text)         (std::wstring(1, 0xE000 + 3) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define RED(text)          (std::wstring(1, 0xE000 + 4) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define MAGENTA(text)      (std::wstring(1, 0xE000 + 5) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define YELLOW(text)       (std::wstring(1, 0xE000 + 6) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define WHITE(text)        (std::wstring(1, 0xE000 + 7) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define GRAY(text)         (std::wstring(1, 0xE000 + 8) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define LIGHTBLUE(text)    (std::wstring(1, 0xE000 + 9) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define LIGHTGREEN(text)   (std::wstring(1, 0xE000 + 10) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define LIGHTCYAN(text)    (std::wstring(1, 0xE000 + 11) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define LIGHTRED(text)     (std::wstring(1, 0xE000 + 12) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define LIGHTMAGENTA(text) (std::wstring(1, 0xE000 + 13) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define LIGHTYELLOW(text)  (std::wstring(1, 0xE000 + 14) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()
#define RESET_COLOR(text)  (std::wstring(1, 0xE000 + 15) + std::wstring(text) + std::wstring(1, 0xE00F)).c_str()

class ConsoleColor
{
	ConsoleColor() = delete;

public:
	static void wcprintf(const wchar_t* format, ...);
};

// Call examples:
// ConsoleColor::wcprintf(L"[\U0000E002 Green Text \U0000E00F] - Default color\n");
// ConsoleColor::wcprintf(L"[%s] - Default color", GREEN(L" Green Text "));
