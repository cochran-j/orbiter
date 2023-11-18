// Copyright (c) Jack Cochran
// Licensed under the MIT License

#ifndef __DINPUT_H
#define __DINPUT_H

#include "Input.h"
#include "Di7frame.h"

#include <memory>
#include <vector>

class DInputKbdDevice;
class DInputJoyDevice;

class DInput : public I_Input {
public:
    DInput();
    ~DInput();

    HRESULT Create (HINSTANCE hInst) override;
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
    std::unique_ptr<CDIFramework7> diframe;
    // These two depend on diframe so it's important they come after.
    std::unique_ptr<DInputKbdDevice> KbdDevice;
    std::unique_ptr<DInputJoyDevice> JoyDevice;
    HWND m_hWnd;
};

class DInputKbdDevice : public I_KbdDevice {
public:
    DInputKbdDevice(CDIFramework7* diframe);

    HRESULT Acquire() override;
    HRESULT Unacquire() override;

    const std::vector<int>& GetKeysImmediate() override;
    const std::vector<KeyEvent>& GetKeysBuffered() override;

private:
    CDIFramework7* diframe;
    std::vector<int> KeysImmediate;
    std::vector<KeyEvent> KeysBuffered;
};

class DInputJoyDevice : public I_JoyDevice {
public:
    DInputJoyDevice(CDIFramework7* diframe);

    HRESULT Acquire() override;
    HRESULT Unacquire() override;

    int GetNumAxes() const override;
    int GetNumHats() const override;
    int GetNumButtons() const override;

    int GetAxis(int axisIdx) override;
    int GetHat(int hatIdx) override;
    bool GetButton(int buttonIdx) override;

    const JoyProp& GetProps() const override;

    HRESULT SetJoystickProperties(const CFG_JOYSTICKPRM& joystickParameters);

private:
    bool PollJoystick(DIJOYSTATE2* js);

private:
    CDIFramework7* diframe;
    JoyProp joyprop;
};


#endif // !__DINPUT_H

