/* Win32RenderWindowFactory.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#include "Win32RenderWindowFactory.h"
#include "ServiceLocator.h"

#include <windows.h>
#include <memory>
#include <string>
#include <string_view>

namespace orbiter {

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Static msg handler which passes messages from the render window to the
// application class.
//----------------------------------------------------------------------------
DLLEXPORT LRESULT CALLBACK WndProc(HWND hWnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam) {


    auto renderWndFact = static_cast<Win32RenderWindowFactory*>
        (GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (renderWndFact) {
        return renderWndFact->RenderWndPRoc(hWnd, uMsg, wParam, lParam);
    } else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

Win32RenderWindowFactory::Win32RenderWindowFactory(HINSTANCE inst)
    : hModule{inst} {

    WNDCLASS wndClass = {
        0,
        Win32RenderWindowFactory::WndProc,
        0,
        0,
        hModule,
        LoadIcon(g_pOrbiter->GetInstance(), MAKEINTRESOURCE(IDI_MAIN_ICON)),
        LoadCursor(NULL, IDC_ARROW),
        (HBRUSH) GetStockObject(WHITE_BRUSH),
        NULL,
        strWndClass
    };

    RegisterClass(&wndClass);
}

HWND Win32RenderWindowFactory::create(int width, int height) {
    auto hwnd = CreateWindow
        (strWndClass,
         "",
         WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         width,
         height,
         0,
         0,
         hModule,
         (LPVOID) this);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(this));

    return hwnd;
}


HWND Win32RenderWindowFactory::createFullscreen() {
    auto hwnd = CreateWindow(strWndClass, "", // Dummy window
        WS_POPUP | WS_EX_TOPMOST | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        10,
        10,
        0,
        0,
        hModule,
        (LPVOID) this);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(this));

    return hwnd;
}

std::string Win32RenderWindowFactory::windowTitle(HWND hwnd) const {
    std::string ret {};
    ret.resize(128);
    GetWindowText(hwnd, ret.data(), 128);
    return ret;
}


void Win32RenderWindowFactory::setWindowTitle(HWND hwnd,
                                              std::string_view newTitle) const {

    SetWindowText(hwnd, newTitle.c_str());
}



LRESULT Win32RenderWindowFactory::RenderWndProc(HWND hWnd,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam) {

    // NOTE(jec):  Combining original GraphicsClient::RenderWndProc()
    // and Orbiter::MsgProc() here.
    //
    // This seems to mainly pass some keyboard and mouse events to the
    // Select and InputBox dialogs, which seem to not be real windows.

    /* TODO(jec):  Global mainloop flags need to work into the main loop. */
    bool bRenderOnce = false;
    /* TODO(jec):  KeepFocus is not for mainloop.  It reflects a "rotation
     *             mode." */
    bool bKeepFocus = false;


    switch (uMsg) {
    case WM_ACTIVATE:
        bActive = (wParam != WA_INACTIVE);
        return 0;

    case WM_CHAR: {
        // make dialogs modal to avoid complications
        auto&& g_input = orbiter::getService()->getInputBox();
        if (g_input->IsActive()) {
            if (g_input->ConsumeKey(uMsg, wParam) != Select::key_ignore) bRenderOnce = true;
            return 0;
        }
        auto&& g_select = orbiter::getService()->getSelectBox();
        if (g_select->IsActive()) {
            if (g_select->ConsumeKey(uMsg, wParam) != Select::key_ignore) bRenderOnce = true;
            return 0;
        }
        }
        break;

    // *** User Keyboard Input ***
    case WM_KEYDOWN: {

        // modifiers
        WORD kmod = 0;
        if (GetKeyState (VK_SHIFT)   & 0x8000) kmod |= 0x01;
        if (GetKeyState (VK_CONTROL) & 0x8000) kmod |= 0x02;

        // make dialogs modal to avoid complications
        auto&& g_input = orbiter::getService()->getInputBox();
        if (g_input && g_input->IsActive()) {
            if (g_input->ConsumeKey (uMsg, wParam, kmod) != Select::key_ignore) bRenderOnce = true;
            return 0;
        }
        auto&& g_select = orbiter::getService()->getSelectBox();
        if (g_select && g_select->IsActive()) {
            if (g_select->ConsumeKey (uMsg, wParam, kmod) != Select::key_ignore) bRenderOnce = true;
            return 0;
        }
        }
        break;

    // Mouse event handler
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        /* TODO(jec):  MouseEvent() et seq to fold into RenderWindowFactory?? */
        /*             More likely, some kind of Mouse Processor module. */
        if (MouseEvent(uMsg, wParam, LOWORD(lParam), HIWORD(lParam))) {
            break; //return 0;
        } break;
    case WM_MOUSEWHEEL: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (!bFullscreen) {
            POINT pt = { x, y };
            ScreenToClient(&pt); // for some reason this message passes screen coordinates
            x = pt.x;
            y = pt.y;
        }
        if (MouseEvent(uMsg, wParam, x, y)) {
            break; //return 0;
        }
        } break;

    case WM_MOUSEMOVE: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        MouseEvent(uMsg, wParam, x, y);
        /* TODO(jec):  Consider factoring this focus management into some
         * event handling module.
         */
        if (!bKeepFocus &&
            orbiter::getService()->getConfig().CfgUIPrm.MouseFocusMode != 0 &&
            GetFocus() != hWnd) {

            if (GetWindowThreadProcessId(hWnd, NULL) == GetWindowThreadPRocessId(GetFocus(), NULL)) {
                SetFocus(hWnd);
            }
        }
        } return 0;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO*) lParam)->ptMinTrackSize.x = 100;
        ((MINMAXINFO*) lParam)->ptMinTrackSize.y = 100;
        break;

    case WM_POWERBROADCAST:
        switch (wParam) {
        case PBT_APMQUERYSUSPEND:
            // At this point, the app should save any data for open
            // network connections, files, etc.., and prepare to go into
            // a suspended mode.
            Freeze (true);
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            // At this point, the app should recover any data, network
            // connections, files, etc.., and resume running from when
            // the app was suspended.
            Freeze (false);
            return TRUE;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case SC_MONITORPOWER:
            // Prevent potential crashes when the monitor powers down
            return 1;

        case IDM_EXIT:
            // Recieved key/menu command to exit render window
            SendMessage (hWnd, WM_CLOSE, 0, 0);
            return 0;
        }
        break;

    case WM_NCHITTEST:
        // Prevent the user from selecting the menu in fullscreen mode
        if (IsFullscreen()) return HTCLIENT;
        break;

        // shutdown options
    case WM_CLOSE:
        getService()->getOrbiter().PreCloseSession();
        DestroyWindow (hWnd);
        return 0;

    case WM_DESTROY:
        getService()->getOrbiter().CloseSession();
        break;
    }
    return DefWindowProc (hWnd, uMsg, wParam, lParam);
}


std::unique_ptr<IRenderWindowFactory> IRenderWindowFactory::instantiate
    (HINSTANCE inst) {
    return std::make_unique<Win32RenderWindowFactory>(inst);
}


}

