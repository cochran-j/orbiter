/* SDLInput.cpp
 *
 * Copyright (c) 2023, Jack Cochran
 * Licensed under the MIT license.
 */

#if CONFIG_SDL_INPUT

#include "SDLInput.h"
#include "Input.h"

#include <memory>
#include <vector>


std::unique_ptr<I_Input> input_CreateSDLInput() {
    return std::make_unique<SDLInput>();
}

SDLInput::SDLInput()
    : KbdDevice{},
      JoyDevice{} {

    return;
}

HRESULT SDLInput::Create(HINSTANCE /*hInst*/) {
    return 0;
}

void SDLInput::Destroy() {
    return;
}

void SDLInput::SetRenderWindow(HWND /*hWnd*/) {
    return;
}

int SDLInput::NumJoyDevices() const {
    return 0;
}

bool SDLInput::CreateKbdDevice() {
    KbdDevice = std::make_unique<SDL_KbdDevice>();
    return true;
}

bool SDLInput::CreateJoyDevice(const CFG_JOYSTICKPRM& /*joystickParmaeters*/) {
    JoyDevice = std::make_unique<SDL_JoyDevice>();
    return true;
}

void SDLInput::DestroyDevices() {
    return;
}

I_KbdDevice& SDLInput::GetKbdDevice() {
    return *KbdDevice;
}

const I_KbdDevice& SDLInput::GetKbdDevice() const {
    return *KbdDevice;
}

I_JoyDevice& SDLInput::GetJoyDevice() {
    return *JoyDevice;
}

const I_JoyDevice& SDLInput::GetJoyDevice() const {
    return *JoyDevice;
}

void SDLInput::OptionChanged(DWORD /*cat*/, DWORD /*item*/) {
    return;
}


SDL_KbdDevice::SDL_KbdDevice()
    : KeysImmediate(0, 256),
      KeysBuffered{} {

    return;
}

HRESULT SDL_KbdDevice::Acquire() {
    return 0;
}

HRESULT SDL_KbdDevice::Unacquire() {
    return 0;
}

const std::vector<int>& SDL_KbdDevice::GetKeysImmediate() {
    return KeysImmediate;
}

const std::vector<SDL_KbdDevice::KeyEvent>& SDL_KbdDevice::GetKeysBuffered() {
    return KeysBuffered;
}


SDL_JoyDevice::SDL_JoyDevice()
    : props{} {

    return;
}

HRESULT SDL_JoyDevice::Acquire() {
    return 0;
}

HRESULT SDL_JoyDevice::Unacquire() {
    return 0;
}

int SDL_JoyDevice::GetNumAxes() const {
    return 0;
}

int SDL_JoyDevice::GetNumHats() const {
    return 0;
}

int SDL_JoyDevice::GetNumButtons() const {
    return 0;
}

int SDL_JoyDevice::GetAxis(int axisIdx) {
    return 0;
}

int SDL_JoyDevice::GetHat(int hatIdx) {
    return 0;
}

bool SDL_JoyDevice::GetButton(int buttonIdx) {
    return 0;
}

const SDL_JoyDevice::JoyProp& SDL_JoyDevice::GetProps() const {
    return props;
}


#endif /* CONFIG_SDL_INPUT */

