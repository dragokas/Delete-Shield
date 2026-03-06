#include "menu.h"
#include "core.h"
#include "path.h"
#include "console_color.h"

#include <cstdio>
#include <conio.h>
#include <filesystem>
#include <iostream>
#include <string>

bool AskEnterPath(const std::wstring& prompt, std::filesystem::path& path)
{
    wprintf(L"%s", prompt.c_str());
    std::wstring input;
    std::getline(std::wcin, input);
    try
    {
        path = Path::NormalizePath(input);
        return true;
    }
    catch (const std::exception& e) {
        printf("Wrong path. %s\n", e.what());
    }
    return false;
}

void ShowKeys()
{
    wprintf(L"Interative mode is ON."
        "\n"
        "\n"
        "Press key to start the action:");
    ConsoleColor::wcprintf(L"\n[%s] - Monitor",   GREEN(L"M"));
    ConsoleColor::wcprintf(L"\n[%s] - Block",     GREEN(L"B"));
    ConsoleColor::wcprintf(L"\n[%s] - Unlock",    GREEN(L"U"));
    ConsoleColor::wcprintf(L"\n[%s] - Exit",      GREEN(L"X"));
    wprintf(L"\n\n");
}

int Menu::Show()
{
    bool running = true;
    std::filesystem::path path;

    while (running)
    {
        ShowKeys();
        int ch = _getwch();

        switch (ch) {
        case L'X':
        case L'x':
            running = false;
            break;

        case L'M':
        case L'm':
            if (AskEnterPath(L"Enter path to start monitoring: ", path)) {
                Core::StartMonitor(path);
            }
            break;

        case L'B':
        case L'b':
            if (AskEnterPath(L"Enter path to start blocking: ", path)) {
                Core::StartBlocker(path);
            }
            break;

        case L'U':
        case L'u':
            if (AskEnterPath(L"Enter path to unlock: ", path)) {
                Core::StartUnloker(path);
            }
            break;

        default:
            break;
        }
    }

	return 0;
}