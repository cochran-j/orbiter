/* SDL2RenderWindowFactory.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 *
 */

#include "SDL2RenderWindowFactory.h"

#include "SDL.h"

#include <windows.h>
#include <memory>
#include <string>
#include <string_view>

namespace orbiter {

SDL2RenderWindowFactory::SDL2RenderWindowFactory(HINSTANCE inst) 
    : hInst{inst} {

    return;
}

HWND SDL2RenderWindowFactory::create(int width, int height) {

    auto win = SDL_CreateWindow
        ("",
         SDL_WINDOWPOS_UNDEFINED,
         SDL_WINDOWPOS_UNDEFINED,
         width,
         height,
         SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    return static_cast<HWND>(win);

}

HWND SDL2RenderWindowFactory::createFullscreen() {
    auto win = SDL_CreateWindow
        ("",
         SDL_WINDOWPOS_UNDEFINED,
         SDL_WINDOWPOS_UNDEFINED,
         0,
         0,
         SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);

    return static_cast<HWND>(win);
}


std::string SDL2RenderWindowFactory::windowTitle(HWND hwnd) const {
    return SDL_GetWindowTitle(reinterpret_cast<SDL_Window*>(hwnd));
}

void SDL2RenderWindowFactory::setWindowTitle(HWND hwnd,
                                             std::string_view newTitle) const {

    SDL_SetWindowTitle(reinterpret_cast<SDL_Window*>(hwnd), newTitle.data());
}

std::unique_ptr<IRenderWindowFactory> IRenderWindowFactory::instantiate
    (HINSTANCE inst) {

    return std::make_unique<SDL2RenderWindowFactory>(inst);
}



}

