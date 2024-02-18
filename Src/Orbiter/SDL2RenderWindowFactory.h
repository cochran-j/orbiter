/* SDL2RenderWindowFactory.h
 * Implements a IRenderWindowFactory using SDL2 backend.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef SRC_ORBITER_SDL2RENDERWINDOWFACTORY_H__
#define SRC_ORBITER_SDL2RENDERWINDOWFACTORY_H__

#include "RenderWindowFactory.h"

#include <windows.h>
#include <string>
#include <string_view>


namespace orbiter {

class SDL2RenderWindowFactory : public IRenderWindowFactory {
public:
    SDL2RenderWindowFactory(HINSTANCE inst);

    virtual HWND create(int width, int height) override;
    virtual HWND createFullscreen() override;

    std::string windowTitle(HWND hwnd) const override;
    void setWindowTitle(HWND hwnd, std::string_view newTitle) const override;

private:
    HINSTANCE hInst;
};



}

#endif /* SRC_ORBITER_SDL2RENDERWINDOWFACTORY_H__ */

