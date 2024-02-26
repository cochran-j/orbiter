/* WinCompat.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#include "WinCompat.h"

#include <windows.h>


#if CONFIG_DXVK /* Proxy for using SDL2 instead of Win32 */

#include "SDL.h"

namespace d3d9client {

bool GetWindowDims(HWND win, RECT* dims) {
    if (!win || !dims) {
        return false;
    }

    auto sdlWin = reinterpret_cast<SDL_Window*>(win);

    int x, y, width, height;
    SDL_GetWindowPosition(sdlWin, &x, &y);
    SDL_GetWindowSizeInPixels(sdlWin, &width, &height);

    dims->left = x;
    dims->right = x + width;
    dims->top = y;
    dims->bottom = y + height;

    return true;
}

bool GetWindowDimsWithBorders(HWND win, RECT* dims) {
    if (!win || !dims) {
        return false;
    }

    auto sdlWin = reinterpret_cast<SDL_Window*>(win);

    int x, y, width, height, left, right, top, bottom;
    SDL_GetWindowPosition(sdlWin, &x, &y);
    SDL_GetWindowSizeInPixels(sdlWin, &width, &height);
    SDL_GetWindowBordersSize(sdlWin,
                             &top, &left,
                             &bottom, &right);

    dims->left = x - left;
    dims->right = x + width + right;
    dims->top = y - top;
    dims->bottom = y + height + bottom;

    return true;
}

bool GetPrimaryScreenSize(int* width, int* height) {
    SDL_DisplayMode dm {};
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0) {
        return false;
    }

    if (width) {
        *width = dm.w;
    }
    if (height) {
        *height = dm.h;
    }
    return true;
}

bool GetPrimaryScreenSizeLessTaskbar(int* width, int* height) {
    /* TODO(jec):  Not clear easiest way to do this x-plat. */
    return GetPrimaryScreenSize(width, height);
}

bool ShowOrHideWindow(HWND win, bool showWin) {
    if (showWin) {
        SDL_ShowWindow(reinterpret_cast<SDL_Window*>(win));
    } else {
        SDL_HideWindow(reinterpret_cast<SDL_Window*>(win));
    }
    return true;
}

void SetClipChildrenVisible(HWND win) {
    

}

bool MoveWindowDims(HWND win,
                    int x, int y,
                    int width, int height) {

    auto sdlWin = reinterpret_cast<SDL_Window*>(win);
    SDL_SetWindowPosition(sdlWin, x, y);
    SDL_SetWindowSize(sdlWin, width, height);
    return true;
}


}

#else /* Win32 wrappers */

namespace d3d9client {

bool GetWindowDims(HWND win, RECT* dims) {
    return GetClientRect(win, dims);
}

bool GetWindowDimsWithBorders(HWND win, RECT* dims) {
    return GetWindowRect(win, dims);
}

bool GetPrimaryScreenSize(int* width, int* height) {

    if (width) {
        *width = GetSystemMetrics(SM_CXSCREEN);
    }
    if (height) {
        *height = GetSystemMetrics(SM_CYSCREEN);
    }
    return true;
}

bool GetPrimaryScreenSizeLessTaskbar(int* width, int* height) {
    RECT rect {};
    SystemParameterInfo(SPI_GETWORKAREA, 0, &rect, 0);
    int x = GetSystemMetrics(SM_CXSCREEN);

    rect.right = x;
    rect.bottom = rect.bottom - rect.top;

    if (width) {
        *width = rect.right - rect.left;
    }
    if (height) {
        *height = rect.bottom - rect.top;
    }

    return true;
}

bool ShowOrHideWindow(HWND win, bool showWin) {

    return ShowWindow(win, showWin ? SW_SHOW : SW_HIDE);
}

void SetClipChildrenVisible(HWND win) {

}

bool MoveWindowDims(HWND win,
                    int x, int y,
                    int width, int height) {

    return MoveWindow(win, x, y, width, height, TRUE);
}


}

#endif

