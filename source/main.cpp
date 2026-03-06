#include "core.h"
#include "command_line.h"
#include "menu.h"
#include "visual.h"

// TODO:
// Async I/O

int wmain(int argc, wchar_t* argv[])
{
    CommandLine::ShowAppName();
    Visual::SetTitle();
    Visual::SetIcon();

    Core::SetupPrivileges();

    if (argc == 1)
    {
        CommandLine::ShowUsage();
        return Menu::Show();
    }

    return CommandLine::Parse(argc, argv);
}