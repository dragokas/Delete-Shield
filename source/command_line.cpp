#include "command_line.h"
#include "core.h"
#include "path.h"
#include "globals.h"
#include "console_color.h"

#include <wchar.h>
#include <filesystem>

int CommandLine::Parse(int argc, wchar_t* argv[])
{
    if (argc != 3)
    {
        wprintf(L"Wrong number of arguments!\n");
        ShowUsage();
        return 1;
    }

    std::filesystem::path rootDir;
    if (!Path::NormalizePath(argv[2], rootDir))
    {
        wprintf(L"Path not exists: %s\n", argv[2]);
        return 1;
    }

    if (wcscmp(argv[1], L"-m") == 0)
    {
        Core::StartMonitor(rootDir);
    }
    else if (wcscmp(argv[1], L"-b") == 0)
    {
        Core::StartBlocker(rootDir);
    }
    else if (wcscmp(argv[1], L"-u") == 0)
    {
        Core::StartUnloker(rootDir);
    }
    else {
        wprintf(L"Wrong arguments!\n\n");
        return 1;
    }
    return 0;
}

void CommandLine::ShowAppName()
{
    ConsoleColor::wcprintf(YELLOW(L"%s v%s by Dragokas\n"), Globals::APP_NAME.c_str(), Globals::APP_VERSION.c_str());
}

void CommandLine::ShowUsage()
{
    auto exe = Path::GetExecutableFilename();
    auto temp = Path::GetTempDirectoryPath();

    wprintf(L"\n"
        "Command line keys:\n"
        "\n"
        "-m <path> | Monitor directory and view CREATE files events.\n"
        "-b <path> | Block DELETE access for newly created files.\n"
        "-u <path> | Unlock DELETE access from any files.\n"
        "\n"
        "Examples:\n"
        "Block DELETE operations for all newly created files in the user's temp directory recursively:\n"
        "%s -b \"%s\"\n"
        "\n", exe.c_str(), temp.c_str());
}