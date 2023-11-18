// Copyright (c) Martin Schweiger
// Licensed under the MIT License

// ========================================================================
// To be linked into all Orbiter addon modules.
// Contains standard module entry point and version information.
// ========================================================================

#include <windows.h>
#include "DllCompat.h"
#include <fstream>
#include <stdio.h>

#ifdef _MSC_VER
#define DLLCLBK extern "C" __declspec(dllexport)
#define OAPIFUNC __declspec(dllimport)
#else
#define DLLCLBK extern "C"
#define OAPIFUNC
#endif

OAPIFUNC void InitLib(HINSTANCE);
typedef void (*DllExitFunc)(HINSTANCE);

static DllExitFunc DLLExit {};
static HINSTANCE hMod {};

void DLLENTER DllEnter() {
    static bool need_init = true;
    if (!need_init) {
        return;
    }
    need_init = false;

    if (!hMod) {
        hMod = DllGetInstance(reinterpret_cast<void*>(DllEnter));
    }
    if (!hMod) {
        return;
    }

    InitLib(hMod);
    DLLExit = (DllExitFunc)GetProcAddress (hMod, "ExitModule");
    if (!DLLExit) DLLExit = (DllExitFunc)GetProcAddress (hMod, "opcDLLExit");
}

void DLLEXIT DllExit() {
    if (DLLExit && hMod) (*DLLExit)(hMod);
}

#ifdef _WIN32
BOOL WINAPI DllMain (HINSTANCE hModule,
					 DWORD ul_reason_for_call,
					 LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
        hMod = hModule;
        DllEnter();
		break;
	case DLL_PROCESS_DETACH:
        DllExit();
		break;
	}
	return TRUE;
}
#endif


int oapiGetModuleVersion ()
{
	static int v = 0;
	if (!v) {
		OAPIFUNC int Date2Int (char *date);
		v = Date2Int ((char*)__DATE__);
	}
	return v;
}

DLLCLBK int GetModuleVersion (void)
{
	return oapiGetModuleVersion();
}

void dummy () {}

