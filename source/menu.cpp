#include "menu.h"
#include "core.h"
#include "path.h"
#include "console_color.h"

#include <cstdio>
#include <conio.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

bool AskEnterPath(const std::wstring& prompt, std::filesystem::path& path, 
                  std::vector<std::wstring>& wildcards)
{
    wprintf(L"%s", prompt.c_str());
    std::wstring input;
    std::getline(std::wcin, input);
    
    // Parse: first token is path, rest are wildcards
    std::wistringstream iss(input);
    std::wstring pathStr;
    
    if (!(iss >> pathStr))
        return false;
    
    wildcards.clear();
    std::wstring wc;
    while (iss >> wc)
    {
        wildcards.push_back(wc);
    }
    
    try
    {
        path = Path::NormalizePath(pathStr);
        if (!wildcards.empty())
        {
            wprintf(L"Filters: ");
            for (const auto& w : wildcards)
                wprintf(L"%s ", w.c_str());
            wprintf(L"\n");
        }
        return true;
    }
    catch (const std::exception& e) {
        wprintf(L"Wrong path. %S\n", e.what());
    }
    return false;
}

void ShowKeys()
{
    wprintf(L"Interactive mode is ON."
        "\n"
        "\n"
        "Press key to start the action:");
    ConsoleColor::wcprintf(L"\n[%s] - Monitor (with optional wildcards)",   GREEN(L"M"));
    ConsoleColor::wcprintf(L"\n[%s] - Block   (with optional wildcards)",   GREEN(L"B"));
    ConsoleColor::wcprintf(L"\n[%s] - Unlock  (wildcards ignored)",         GREEN(L"U"));
    ConsoleColor::wcprintf(L"\n[%s] - Exit",                                 GREEN(L"X"));
    wprintf(L"\n\n");
    wprintf(L"Usage for Monitor/Block:\n");
    wprintf(L"  <path> [wildcard1 wildcard2 ...]\n");
    wprintf(L"  Example: C:\\Temp *.exe *.dll\n");
    wprintf(L"\n");
}

int Menu::Show()
{
    bool running = true;
    std::filesystem::path path;
    std::vector<std::wstring> wildcards;

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
            if (AskEnterPath(L"Enter path [and wildcards]: ", path, wildcards)) {
                Core::StartMonitor(path, wildcards);
            }
            break;

        case L'B':
        case L'b':
            if (AskEnterPath(L"Enter path [and wildcards]: ", path, wildcards)) {
                Core::StartBlocker(path, wildcards);
            }
            break;

        case L'U':
        case L'u':
            wildcards.clear(); // unlock doesn't support wildcards
            if (AskEnterPath(L"Enter path to unlock (wildcards ignored): ", path, wildcards)) {
                if (!wildcards.empty())
                {
                    wprintf(L"Warning: Wildcards are ignored for unlock operation.\n");
                }
                Core::StartUnloker(path);
            }
            break;

        default:
            break;
        }
    }

	return 0;
}