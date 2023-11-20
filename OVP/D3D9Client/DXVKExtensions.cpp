/* DXVKExtensions.cpp
 * Extension functions for DXVK.
 *
 * Copyright (c) 2023, Jack Cochran
 * Licensed under the MIT License.
 */

#if CONFIG_DXVK

#include "DXVKExtensions.h"

#include <SDL.h>
#include "Log.h"

namespace DXVK {

bool init_DXVK() {
    int result = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (result != 0) {
        LogAlw("DXVK VIDEO:  SDL_InitSubsystem() failed, %s", SDL_GetError());
    }

    return (result == 0);
}

void exit_DXVK() {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


}

#endif /* ORBITER_DXVK */
