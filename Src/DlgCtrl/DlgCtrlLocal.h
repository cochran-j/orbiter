// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#ifndef __DLGCTRLLOCAL_H
#define __DLGCTRLLOCAL_H

#include <windows.h>

typedef struct {
    /* DLG
	HPEN hPen1, hPen2;
	HBRUSH hBrush1, hBrush2;
    */
} GDIRES;

void RegisterPropertyList (HINSTANCE hInst);
void UnregisterPropertyList (HINSTANCE hInst);

#endif // !__DLGCTRLLOCAL_H
