// Copyright (c) Martin Schweiger
// Licensed under the MIT License

// =======================================================================
// DirectInput user interface class
// =======================================================================

#if CONFIG_DIRECT_INPUT

#include "Input.h"
#include "DInput.h"
#include "Config.h"
#include "Log.h"

#include <memory>
#include <vector>
#include <array>
#include <algorithm>


std::unique_ptr<I_Input> input_CreateDirectInput() {
    return std::make_unique<DInput>();
}

DInput::DInput ()
{
	diframe = nullptr;
    KbdDevice = nullptr;
    JoyDevice = nullptr;
	m_hWnd = nullptr;
}

DInput::~DInput ()
{
	Destroy();
}

HRESULT DInput::Create (HINSTANCE hInst)
{
    // OUT_OF_MEMORY code not returned--expect to throw std::bad_alloc when
    // out of memory.
	diframe = std::make_unique<CDIFramework7>();
    return diframe->Create (hInst);
}

void DInput::Destroy ()
{
    // Destroy in reverse order of construction.
    JoyDevice.reset();
    KbdDevice.reset();
    diframe.reset();
}

void DInput::SetRenderWindow(HWND hWnd)
{
	if (diframe)
		diframe->DestroyDevices();
	m_hWnd = hWnd;
}

int DInput::NumJoyDevices() const {
    return diframe->NumJoysticks();
}

bool DInput::CreateKbdDevice()
{
	if (!m_hWnd) return false; // no render window defined

	if (FAILED (diframe->CreateKbdDevice (m_hWnd))) {
		LOGOUT("ERROR: Could not create keyboard device");
		return false; // we need the keyboard, so give up
	}
    KbdDevice = std::make_unique<DInputKbdDevice>(diframe.get());
	GetKbdDevice()->Acquire();

	return true;
}

bool DInput::CreateJoyDevice(const CFG_JOYSTICKPRM& joystickParameters)
{
	if (!m_hWnd) return false; // no render window defined
	if (!joystickParameters.Joy_idx) return false; // no joystick requested

	if (FAILED (diframe->CreateJoyDevice (m_hWnd, joystickParameters.Joy_idx-1))) {
		LOGOUT_ERR("Could not create joystick device");
		return false;
	}

    JoyDevice = std::make_unique<DInputJoyDevice>(diframe.get());
	HRESULT hr = JoyDevice->Acquire();

    // TODO(jec):  This error handling looks improvable.
	if (hr == DIERR_OTHERAPPHASPRIO) {
		Sleep(1000);
		hr = GetJoyDevice()->Acquire();
	}
	switch (hr) {
	case DIERR_OTHERAPPHASPRIO:
		hr = DI_OK;
		break;
	}

	if (JoyDevice->SetJoystickProperties (joystickParameters) != DI_OK) {
		LOGOUT_ERR("Could not set joystick properties");
		return false;
	}


	return true;
}

void DInput::DestroyDevices ()
{
	diframe->DestroyDevices();
}

I_KbdDevice& DInput::GetKbdDevice() {
    return *KbdDevice;
}

const I_KbdDevice& DInput::GetKbdDevice() const {
    return *KbdDevice;
}

I_JoyDevice& DInput::GetJoyDevice() {
    return *JoyDevice;
}

const I_JoyDevice& DInput::GetJoyDevice() const {
    return *JoyDevice;
}

void DInput::OptionChanged(DWORD cat, DWORD item)
{
	if (cat == OPTCAT_JOYSTICK) {
		switch (item) {
		case OPTITEM_JOYSTICK_DEVICE:
			diframe->DestroyJoyDevice();
			CreateJoyDevice();
            // Presumably also need to SetProperties here as well.
			break;
		case OPTITEM_JOYSTICK_PARAM:
            // TODO:  Get the joystick parameters over here.
            /*
			SetJoystickProperties();
            */
			break;
		}
	}
}

DInputKbdDevice::DInputKbdDevice(CDIFramework7* diframe_)
    : diframe{diframe_},
      KeysImmediate{},
      KeysBuffered{} {

    KeysImmediate.resize(256);
    KeysBuffered.reserve(10); // ??
}

HRESULT DInputKbdDevice::Acquire() {
    auto dev = diframe->GetKbdDevice();
    if (!dev) {
        return S_FALSE;
    }

    return dev->Acquire();
}

HRESULT DInputKbdDevice::Unacquire() {
    auto dev = diframe->GetKbdDevice();
    if (!dev) {
        return S_FALSE;
    }

    return dev->Unacquire();
}

const std::vector<int>& DInputKbdDevice::GetKeysImmediate() {
    auto dev = diframe->GetKbdDevice();
    if (!dev) {
        return S_FALSE;
    }

    std::array<char, 256> keybuf;
    HRESULT hr = dev->GetDeviceState(keybuf.size(), keybuf.data());
    if ((hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) && SUCCEEDED(dev->Acquire())) {
        hr = dev->GetDeviceState(keybuf.size(), keybuf.data());
    }

    if (SUCCEEDED(hr)) {
        // TODO:  Keys-ored in and consumed??
        std::copy(keybuf.cbegin(), keybuf.cend(), KeysImmediate.begin());
    }

    return KeysImmediate;
}

const std::vector<KeyEvent>& DInputKbdDevice::GetKeysBuffered() {
    auto dev = diframe->GetKbdDevice();
    if (!dev) {
        return S_FALSE;
    }

    std::array<DIDEVICEOBJECTDATA, 10> dod;
    DWORD dwItems = dod.size();

    HRESULT hr = dev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
                                    dod,
                                    &dwItems,
                                    0);
    if ((hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) && SUCCEEDED(dev->Acquire())) {
        hr = dev->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
                                dod,
                                &dwItems,
                                0);
    }

    if (SUCCEEDED(hr)) {
        KeysBuffered.resize(dwItems);
        for (std::size_t i = 0; i < dwItems; ++i) {
            bool isKeydown = ((dod[i].dwData & 0x80) != 0);
            DWORD key = dod[i].dwOfs;

            KeysBuffered[i].type = isKeydown ? Key_EventType::KeyDown : Key_EventType::KeyUp;
            KeysBuffered[i].key = key;
        }
    }

    return KeysBuffered;
}


DInputJoyDevice::DInputJoyDevice(CDIFramework7* diframe_) 
    : diframe{diframe_},
      joyprop{} { }

HRESULT DInputJoyDevice::Acquire() {
    auto dev = diframe->GetJoyDevice();
    if (!dev) {
        return S_FALSE;
    }

    return dev->Acquire();
}

HRESULT DInputJoyDevice::Unacquire() {
    auto dev = diframe->GetJoyDevice();
    if (!dev) {
        return S_FALSE;
    }

    return dev->Unacquire();
}

int DInputJoyDevice::GetNumAxes() const {
    DIDEVCAPS devcaps {};
    LPDIRECTINPUTDEVICE dev = divframe->GetJoyDevice();
    if (!dev) {
        return 0;
    }

    if (dev->GetCapabilities(&devcaps) != DI_OK) {
        return 0;
    }

    return devcaps.dwAxes;
}

int DInputJoyDevice::GetNumHats() const {
    DIDEVCAPS devcaps {};
    LPDIRECTINPUTDEVICE dev = divframe->GetJoyDevice();
    if (!dev) {
        return 0;
    }

    if (dev->GetCapabilities(&devcaps) != DI_OK) {
        return 0;
    }

    return devcaps.dwPOVs;
}

int DInputJoyDevice::GetNumButtons() const {
    DIDEVCAPS devcaps {};
    LPDIRECTINPUTDEVICE dev = divframe->GetJoyDevice();
    if (!dev) {
        return 0;
    }

    if (dev->GetCapabilities(&devcaps) != DI_OK) {
        return 0;
    }

    return devcaps.dwButtons;
}

int DInputJoyDevice::GetAxis(int axisIdx) {
    DIJOYSTATE2 js {};
    if (!PollJoystick(&js)) {
        return 0;
    }

    switch (axisIdx) {
    case 0:
        return js.lX;

    case 1:
        return js.lY;

    case 2:
        return js.lZ;

    case 3:
        return js.lRx;

    case 4:
        return js.lRy;

    case 5:
        return js.lRz;

    case 6:
        return js.rglSlider[0];

    case 7:
        return js.rglSlider[1];

    default:
        return 0;
    }
}

int DInputJoyDevice::GetHat(int hatIdx) {
    DIJOYSTATE2 js {};
    if (!PollJoystick(&js)) {
        return 0;
    }

    if ((hatIdx >= 0) && (hatIdx < 4)) {
        return static_cast<int>(js.rgdwPOV[hatIdx]);
    } else {
        return 0;
    }
}

bool DInputJoyDevice::GetButton(int buttonIdx) {
    DIJOYSTATE2 js {};
    if (!PollJoystick(&js)) {
        return false;
    }

    if ((buttonIdx >=0) && (buttonIdx < 128)) {
        return js.rgbButtons[buttonIdx] != 0;
    } else {
        return false;
    }
}

bool DInputJoyDevice::PollJoystick (DIJOYSTATE2 *js)
{
	// todo: return joystick data in device-independent format
	//       allow collecting data from more than one joystick

	LPDIRECTINPUTDEVICE8 dev = diframe->GetJoyDevice();
	if (!dev) return false;
	HRESULT hr = dev->Poll();
	//if (hr == DI_OK || hr == DI_NOEFFECT)     // ignore error flag from poll. appears to occasionally return DIERR_UNPLUGGED
		hr = dev->GetDeviceState (sizeof(DIJOYSTATE2), js);
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
			if (SUCCEEDED(dev->Acquire())) {
				dev->Poll();
				hr = dev->GetDeviceState(sizeof(DIJOYSTATE2), js);
			}
		}
	return (hr == S_OK);
}

const JoyProp& DInputJoyDevice::GetProps() const {
    return joyprop;
}

HRESULT DInputJoyDevice::SetJoystickProperties
    (const CFG_JOYSTICKPRM& joystickParameters)
{
	LPDIRECTINPUTDEVICE8 dev = diframe->GetJoyDevice();
	if (!dev) return DI_OK;

	HRESULT hr;
	DIPROPRANGE diprg;
	DIPROPDWORD diprw;
	joyprop.bRudder = false;
	joyprop.bThrottle = false;
	Config *pcfg = orbiter->Cfg();

	// x-axis range
	diprg.diph.dwSize       = sizeof (diprg);
	diprg.diph.dwHeaderSize = sizeof (diprg.diph);
	diprg.diph.dwObj        = DIJOFS_X;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = -1000;
	diprg.lMax              = +1000;
	if ((hr = dev->SetProperty (DIPROP_RANGE, &diprg.diph)) != DI_OK)
		return hr;

	// x-axis deadzone
	diprw.diph.dwSize       = sizeof (diprw);
	diprw.diph.dwHeaderSize = sizeof (diprw.diph);
	diprw.diph.dwObj        = DIJOFS_X;
	diprw.diph.dwHow        = DIPH_BYOFFSET;
	diprw.dwData            = joystickParameters.Deadzone;
	if ((hr = dev->SetProperty (DIPROP_DEADZONE, &diprw.diph)) != DI_OK)
		return hr;

	// y-axis range
	diprg.diph.dwSize       = sizeof (diprg);
	diprg.diph.dwHeaderSize = sizeof (diprg.diph);
	diprg.diph.dwObj        = DIJOFS_Y;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = -1000;
	diprg.lMax              = +1000;
	if ((hr = dev->SetProperty (DIPROP_RANGE, &diprg.diph)) != DI_OK)
		return hr;

	// y-axis deadzone
	diprw.diph.dwSize       = sizeof (diprw);
	diprw.diph.dwHeaderSize = sizeof (diprw.diph);
	diprw.diph.dwObj        = DIJOFS_Y;
	diprw.diph.dwHow        = DIPH_BYOFFSET;
	diprw.dwData            = joystickParameters.Deadzone;
	if ((hr = dev->SetProperty (DIPROP_DEADZONE, &diprw.diph)) != DI_OK)
		return hr;

	joyprop.bRudder = true;
	joyprop.bThrottle = true;

	diprg.diph.dwSize       = sizeof (diprg);
	diprg.diph.dwHeaderSize = sizeof (diprg.diph);
	diprg.diph.dwObj        = DIJOFS_RZ;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = -1000;
	diprg.lMax              = +1000;
	if (dev->SetProperty (DIPROP_RANGE, &diprg.diph) != DI_OK)
		joyprop.bRudder = false;

	diprw.diph.dwSize       = sizeof (diprw);
	diprw.diph.dwHeaderSize = sizeof (diprw.diph);
	diprw.diph.dwObj        = DIJOFS_RZ;
	diprw.diph.dwHow        = DIPH_BYOFFSET;
	diprw.dwData            = joystickParameters.Deadzone;
	if (dev->SetProperty (DIPROP_DEADZONE, &diprw.diph) != DI_OK)
		joyprop.bRudder = false;

	// z-axis range (throttle)
	DWORD thaxis;
	DIJOYSTATE2 js2;
	switch (joystickParameters.ThrottleAxis) {
	case 1:
		LOGOUT ("Joystick throttle: Z-AXIS");
		thaxis = DIJOFS_Z;
		joyprop.ThrottleOfs = (BYTE*)&js2.lZ - (BYTE*)&js2;
		break;
	case 2:
		LOGOUT ("Joystick throttle: SLIDER 0");
		thaxis = DIJOFS_SLIDER(0);
		joyprop.ThrottleOfs = (BYTE*)&js2.rglSlider[0] - (BYTE*)&js2;
		break;
	case 3:
		LOGOUT ("Joystick throttle: SLIDER 1");
		thaxis = DIJOFS_SLIDER(1);
		joyprop.ThrottleOfs = (BYTE*)&js2.rglSlider[1] - (BYTE*)&js2;
		break;
	default:
		joyprop.bThrottle = false;
		LOGOUT ("Joystick throttle disabled by user");
		return DI_OK;
	}

	diprg.diph.dwSize       = sizeof (diprg);
	diprg.diph.dwHeaderSize = sizeof (diprg.diph);
	diprg.diph.dwObj        = thaxis;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = -1000;
	diprg.lMax              = 0;
	if ((hr = dev->SetProperty (DIPROP_RANGE, &diprg.diph)) != DI_OK) {
		joyprop.bThrottle = false;
		LOGOUT("No joystick throttle control detected");
		LOGOUT_DIERR(hr);
		return DI_OK;
	}
	LOGOUT("Joystick throttle control detected");

	// throttle saturation at extreme ends
	diprw.diph.dwSize       = sizeof (diprw);
	diprw.diph.dwHeaderSize = sizeof (diprw.diph);
	diprw.diph.dwObj        = thaxis;
	diprw.diph.dwHow        = DIPH_BYOFFSET;
	diprw.dwData            = joystickParameters.ThrottleSaturation;
	if (dev->SetProperty (DIPROP_SATURATION, &diprw.diph) != DI_OK) {
		LOGOUT_ERR("Setting joystick throttle saturation failed");
	}
	return DI_OK;
}

#endif /* CONFIG_DIRECT_INPUT */
