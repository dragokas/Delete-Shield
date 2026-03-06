#include "visual.h"
#include "resource.h"
#include "globals.h"

#include <windows.h>

void Visual::SetIcon()
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

void Visual::SetTitle()
{
    SetConsoleTitleW(Globals::APP_NAME.c_str());
}