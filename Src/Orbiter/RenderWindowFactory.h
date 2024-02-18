/* RenderWindowFactory.h
 * Handles creating render windows.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef SRC_ORBITER_RENDERWINDOWFACTORY_H__
#define SRC_ORBITER_RENDERWINDOWFACTORY_H__

#include <windows.h>
#include <memory>
#include <string>
#include <string_view>

namespace orbiter {

class IRenderWindowFactory {
public:
    static std::unique_ptr<IRenderWindowFactory> instantiate(HINSTANCE inst);

    virtual ~IRenderWindowFactory() {}

    virtual HWND create(int width, int height) = 0;
    virtual HWND createFullscreen() = 0;

    virtual std::string windowTitle(HWND hwnd) const = 0;
    virtual void setWindowTitle(HWND hwnd, std::string_view newTitle) const = 0;
};


}

#endif /* SRC_ORBITER_RENDERWINDOWFACTORY_H__ */

