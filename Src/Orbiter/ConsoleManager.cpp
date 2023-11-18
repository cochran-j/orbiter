#include "ConsoleManager.h"

#include <windows.h>

bool ConsoleManager::IsConsoleExclusive(void) {
    DWORD pids[2];
    /* TODO(jec):  Win32 console stuff
    DWORD num_pids = GetConsoleProcessList(pids, 2);
    return num_pids <= 1;
    */
    return false;
}

void ConsoleManager::ShowConsole(bool show)
{
    /* TODO(jec)
    HWND wnd = GetConsoleWindow();
    if (wnd)
        ShowWindow(wnd, show ? SW_SHOW : SW_HIDE);
    */
}
