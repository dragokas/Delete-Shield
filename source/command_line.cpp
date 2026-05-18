#include "command_line.h"
#include "core.h"
#include "path.h"
#include "globals.h"
#include "console_color.h"

#include <wchar.h>
#include <filesystem>

static bool ParseArguments(int argc, wchar_t* argv[], CommandLine::ParsedArgs& args)
{
    if (argc < 3)
        return false;
    
    if (argv[1][0] != L'-' || wcslen(argv[1]) != 2)
        return false;
    
    args.mode = argv[1][1];
    
    std::filesystem::path rootDir;
    if (!Path::NormalizePath(argv[2], rootDir))
    {
        wprintf(L"Path not exists: %s\n", argv[2]);
        return false;
    }
    args.path = rootDir.wstring();
    
    // Собираем wildcard'ы из оставшихся аргументов
    for (int i = 3; i < argc; i++)
    {
        args.wildcards.push_back(argv[i]);
    }
    
    return true;
}

int CommandLine::Parse(int argc, wchar_t* argv[])
{
    ParsedArgs args;
    
    if (!ParseArguments(argc, argv, args))
    {
        wprintf(L"Wrong arguments!\n");
        ShowUsage();
        return 1;
    }
    
    // Выводим информацию о фильтрах, если они заданы
    if (!args.wildcards.empty())
    {
        wprintf(L"Filtering by patterns: ");
        for (size_t i = 0; i < args.wildcards.size(); i++)
        {
            wprintf(L"%s%s", args.wildcards[i].c_str(), 
                    i < args.wildcards.size() - 1 ? L", " : L"\n");
        }
        wprintf(L"\n");
    }
    
    switch (args.mode)
    {
    case L'm':
        Core::StartMonitor(args.path, args.wildcards);
        break;
        
    case L'b':
        Core::StartBlocker(args.path, args.wildcards);
        break;
        
    case L'u':
        if (!args.wildcards.empty())
        {
            wprintf(L"Warning: Wildcards are ignored for unlock operation.\n");
        }
        Core::StartUnloker(args.path);
        break;
        
    default:
        wprintf(L"Wrong arguments! Unknown mode: -%c\n\n", args.mode);
        ShowUsage();
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
        "-m <path> [wildcards...] | Monitor directory and view CREATE files events.\n"
        "-b <path> [wildcards...] | Block DELETE access for newly created files.\n"
        "-u <path>                | Unlock DELETE access from any files.\n"
        "\n"
        "Wildcard patterns (case-insensitive, supports * and ?):\n"
        "  *.tmp     - all .tmp files\n"
        "  test*.txt - files starting with 'test' and ending with .txt\n"
        "  file?.log - file1.log, file2.log, etc.\n"
        "\n"
        "Examples:\n"
        "  Block all newly created .exe and .dll files in temp directory:\n"
        "    %s -b \"%s\" *.exe *.dll\n"
        "\n"
        "  Monitor only .tmp and .log files:\n"
        "    %s -m \"C:\\Temp\" *.tmp *.log\n"
        "\n"
        "  Unlock everything (no wildcards needed):\n"
        "    %s -u \"C:\\Protected\"\n"
        "\n", 
        exe.c_str(), temp.c_str(),
        exe.c_str(),
        exe.c_str());
}