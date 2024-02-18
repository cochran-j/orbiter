/* Win32RenderWindowFactory.h
 * Implements IRenderWindowFactory using CreateWindow().
 * Useful for GDI or true DirectX rendering.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef SRC_ORBITER_WIN32RENDERWINDOWFACTORY_H__
#define SRC_ORBITER_WIN32RENDERWINDOWFACTORY_H__

#include "RenderWindowFactory.h"

#include <windows.h>
#include <memory>
#include <string>
#include <string_view>

namespace orbiter {

class Win32RenderWindowFactory {
public:
    Win32RenderWindowFactory(HINSTANCE inst);

    HWND create(int width, int height) override;
    HWND createFullscreen() override;

    std::string windowTitle(HWND hwnd) const override;
    void setWindowTitle(HWND hwnd, std::string_view newTitle) const override;

private:
    LRESULT RenderWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static const char *const strWndClass = "Orbiter Render Window";

    HINSTANCE hModule;
};


}

#endif /* SRC_ORBITER_WIN32RENDERWINDOWFACTORY_H__ */

