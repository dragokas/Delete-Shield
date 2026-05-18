#include "console.h"
#include "resource.h"
#include "globals.h"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

void Console::SetIcon()
{
    HWND consoleWnd = GetConsoleWindow();
    if (consoleWnd) {
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
        if (hIcon) {
            SendMessage(consoleWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessage(consoleWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        }
    }
}

void Console::SetTitle()
{
    SetConsoleTitleW(Globals::APP_NAME.c_str());
}

void Console::SetUnicode()
{
    (void)_setmode(_fileno(stdout), _O_U16TEXT);
    (void)_setmode(_fileno(stderr), _O_U16TEXT);
}

void Console::WaitKey(int virtualCode)
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

    DWORD mode = 0;
    GetConsoleMode(hInput, &mode);

    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;

    SetConsoleMode(hInput, mode);

    INPUT_RECORD record{};
    DWORD read = 0;

    while (true)
    {
        ReadConsoleInput(
            hInput,
            &record,
            1,
            &read);

        if (record.EventType == KEY_EVENT)
        {
            KEY_EVENT_RECORD& key = record.Event.KeyEvent;

            if (key.bKeyDown)
            {
                if (key.wVirtualKeyCode == virtualCode)
                {
                    break;
                }
            }
        }
    }
}
