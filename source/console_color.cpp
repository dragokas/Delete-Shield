#include "console_color.h"

#include <windows.h>
#include <string>
#include <cstdarg>

constexpr WORD COLOR_DEFAULT = 7;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void writeChunk(HANDLE h, const wchar_t* text, size_t len)
{
    DWORD written;
    WriteConsoleW(h, text, (DWORD)len, &written, nullptr);
}

void printColored(const wchar_t* text)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hConsole, &info);

    WORD defaultColor = info.wAttributes;

    const wchar_t* p = text;
    const wchar_t* chunkStart = p;

    while (*p)
    {
        wchar_t c = *p;

        if (c >= 0xE001 && c <= 0xE00F)
        {
            if (p > chunkStart)
                writeChunk(hConsole, chunkStart, p - chunkStart);

            WORD color = (WORD)(c - 0xE000);

            if (color == 0x0F)
                SetConsoleTextAttribute(hConsole, defaultColor);
            else
                SetConsoleTextAttribute(hConsole, color);

            chunkStart = p + 1;
        }

        p++;
    }

    if (p > chunkStart)
        writeChunk(hConsole, chunkStart, p - chunkStart);

    SetConsoleTextAttribute(hConsole, defaultColor);
}

void ConsoleColor::wcprintf(const wchar_t* format, ...)
{
    wchar_t buffer[2048];

    va_list args;
    va_start(args, format);
    vswprintf(buffer, 2048, format, args);
    va_end(args);

    printColored(buffer);
}