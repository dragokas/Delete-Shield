#include "core.h"
#include "command_line.h"
#include "menu.h"
#include "console.h"

int wmain(int argc, wchar_t* argv[])
{
    CommandLine::ShowAppName();
    Console::SetUnicode();
    Console::SetTitle();
    Console::SetIcon();

    Core::SetupPrivileges();

    if (argc == 1)
    {
        CommandLine::ShowUsage();
        wprintf(L"\n");
        return Menu::Show();
    }

    return CommandLine::Parse(argc, argv);
}