/* SDLInput.h
 * SDL implementation for keyboard and joystick direct input.
 *
 * Copyright (c) 2023, Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef __SDLINPUT_H
#define __SDLINPUT_H

#include "Input.h"

#include <memory>
#include <vector>

class SDL_KbdDevice;
class SDL_JoyDevice;

class SDLInput : public I_Input {
public:
    SDLInput();

    HRESULT Create(HINSTANCE hInst) override;
    void Destroy() override;

    void SetRenderWindow(HWND hWnd) override;

    int NumJoyDevices() const override;

    bool CreateKbdDevice() override;
    bool CreateJoyDevice(const CFG_JOYSTICKPRM& joystickParameters) override;
    void DestroyDevices() override;

    I_KbdDevice& GetKbdDevice() override;
    const I_KbdDevice& GetKbdDevice() const override;
    I_JoyDevice& GetJoyDevice() override;
    const I_JoyDevice& GetJoyDevice() const override;

    void OptionChanged(DWORD cat, DWORD item) override;

private:
    std::unique_ptr<SDL_KbdDevice> KbdDevice;
    std::unique_ptr<SDL_JoyDevice> JoyDevice;
};

class SDL_KbdDevice : public I_KbdDevice {
public:
    SDL_KbdDevice();

    HRESULT Acquire() override;
    HRESULT Unacquire() override;

    const std::vector<int>& GetKeysImmediate() override;
    const std::vector<KeyEvent>& GetKeysBuffered() override;

private:
    std::vector<int> KeysImmediate;
    std::vector<KeyEvent> KeysBuffered;
};

class SDL_JoyDevice : public I_JoyDevice {
public:
    SDL_JoyDevice();

    HRESULT Acquire() override;
    HRESULT Unacquire() override;

    int GetNumAxes() const override;
    int GetNumHats() const override;
    int GetNumButtons() const override;

    int GetAxis(int axisIdx) override;
    int GetHat(int hatIdx) override;
    bool GetButton(int buttonIdx) override;

    const JoyProp& GetProps() const override;

private:
    JoyProp props;
};

#endif /* __SDLINPUT_H */

