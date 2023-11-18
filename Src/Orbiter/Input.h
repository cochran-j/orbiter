// Copyright (c) Martin Schweiger
// Licensed under the MIT License

// =======================================================================
// DirectInput user interface class
// =======================================================================

#ifndef __INPUT_H
#define __INPUT_H

#include <windows.h> // HRSULT, HINSTANCE
#include <vector>
#include <memory>

#include "Config.h"

class I_KbdDevice;
class I_JoyDevice;

class I_Input {
public:
    virtual ~I_Input() {};

    virtual HRESULT Create(HINSTANCE hInst) = 0;
    virtual void Destroy() = 0;

    virtual void SetRenderWindow(HWND hWnd) = 0;

    virtual int NumJoyDevices() const = 0;

    virtual bool CreateKbdDevice() = 0;
    virtual bool CreateJoyDevice(const CFG_JOYSTICKPRM& joystickParameters) = 0;
    virtual void DestroyDevices() = 0;

    virtual I_KbdDevice& GetKbdDevice() = 0;
    virtual const I_KbdDevice& GetKbdDevice() const = 0;
    virtual I_JoyDevice& GetJoyDevice() = 0;
    virtual const I_JoyDevice& GetJoyDevice() const = 0;

    virtual void OptionChanged(DWORD cat, DWORD item) = 0;
};

class I_KbdDevice {
public:
    enum class Key_EventType {
        KeyUp = 0,
        KeyDown = 1
    };

    struct KeyEvent {
        Key_EventType type;
        int key;
    };

    virtual ~I_KbdDevice() {};

    // Modeled on IDirectInputDevice8
    virtual HRESULT Acquire() = 0;
    virtual HRESULT Unacquire() = 0;

    virtual const std::vector<int>& GetKeysImmediate() = 0;
    virtual const std::vector<KeyEvent>& GetKeysBuffered() = 0;
};

class I_JoyDevice {
public:
   	struct JoyProp {
		bool bThrottle;  // joystick has throttle control
		bool bRudder;    // joystick has rudder control
		int ThrottleAxisIdx; // throttle axis index
	};

    virtual ~I_JoyDevice() {};

    // Modeled on IDirectInputDevice8
    virtual HRESULT Acquire() = 0;
    virtual HRESULT Unacquire() = 0;

    virtual int GetNumAxes() const = 0;
    virtual int GetNumHats() const = 0;
    virtual int GetNumButtons() const = 0;

    virtual int GetAxis(int axisIdx) = 0;
    // centi-degrees clockwise from up
    virtual int GetHat(int hatIdx) = 0;
    virtual bool GetButton(int buttonIdx) = 0;

    virtual const JoyProp& GetProps() const = 0;
};

std::unique_ptr<I_Input> input_CreateDirectInput();
std::unique_ptr<I_Input> input_CreateSDLInput();



#endif // !__INPUT_H
