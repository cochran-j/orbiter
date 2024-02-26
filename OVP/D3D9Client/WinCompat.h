/* WinCompat.h
 * Window function wrappers which point either to Win32 API or SDL equivalents.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef OVP_D3D9CLIENT_WINCOMPAT_H__
#define OVP_D3D9CLIENT_WINCOMPAT_H__

#include <windows.h>

namespace d3d9client {

/* TODO:  Move HWND and related definitions here? */

/* GetClientRect() */
bool GetWindowDims(HWND win, RECT* dims);

/* GetWindowRect() */
bool GetWindowDimsWithBorders(HWND win, RECT* dims);

/* GetSystemMetrics() */
bool GetPrimaryScreenSize(int* width, int* height);
bool GetPrimaryScreenSizeLessTaskbar(int* width, int *height);

/* ShowWindow() */
bool ShowOrHideWindow(HWND win, bool showWin);

/* MoveWindow / SetWindowPos */
bool MoveWindowDims(HWND win, int x, int y, int width, int height);

/* Set style */
void SetClipChildrenVisible(HWND win);


}

#endif /* OVP_D3D9CLIENT_WINCOMPAT_H__ */

