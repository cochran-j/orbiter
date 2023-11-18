// Copyright (c) Martin Schweiger
// Licensed under the MIT License

// ======================================================================
// Custom function selection dialog
// ======================================================================

#define STRICT 1

#include "DlgFunction.h"
#include "Orbiter.h"
/* TODO(jec)
#include "Resource.h"
#include "Resource2.h"
*/

extern Orbiter *g_pOrbiter;
extern HELPCONTEXT DefHelpContext;

// ======================================================================

DlgFunction::DlgFunction (HINSTANCE hInstance, HWND hParent, void *context)
: DialogWin (hInstance, hParent, /* TODO(jec) IDD_CUSTOMCMD*/ 0, 0, 0, context)
{
}

// ======================================================================

void DlgFunction::ScanFunctions (HWND hDlg)
{
	DWORD i, idx;
    /* TODO(jec)
	SendDlgItemMessage (hDlg, IDC_CUSTOMCMD_LIST, LB_RESETCONTENT, 0, 0);
    */
	for (i = 0; i < g_pOrbiter->ncustomcmd; i++) {
        /* TODO(jec)
		idx = SendDlgItemMessage (hDlg, IDC_CUSTOMCMD_LIST, LB_ADDSTRING, 0, (LPARAM)g_pOrbiter->customcmd[i].label);
        */
    }
    /* TODO(jec)
	if (i) SendDlgItemMessage (hDlg, IDC_CUSTOMCMD_LIST, LB_SETCURSEL, 0, 0);
    */
}

// ======================================================================

void DlgFunction::RunFunction (HWND hDlg)
{
    /* TODO(jec)
	int idx = SendDlgItemMessage (hDlg, IDC_CUSTOMCMD_LIST, LB_GETCURSEL, 0, 0);
	if (idx != LB_ERR) g_pOrbiter->customcmd[idx].func (g_pOrbiter->customcmd[idx].context);
    */
}

// ======================================================================

void DlgFunction::ShowDescription (HWND hDlg)
{
    /* TODO(jec)
	int idx = SendDlgItemMessage (hDlg, IDC_CUSTOMCMD_LIST, LB_GETCURSEL, 0, 0);
	if (idx != LB_ERR)
		SetWindowText (GetDlgItem (hDlg, IDC_CUSTOMCMD_DESCR), g_pOrbiter->customcmd[idx].desc);
    */
}

// ======================================================================

BOOL DlgFunction::OnInitDialog (HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	ScanFunctions (hDlg);
	ShowDescription (hDlg);
	return TRUE;
}

// ======================================================================

BOOL DlgFunction::OnCommand (HWND hDlg, WORD id, WORD code, HWND hControl)
{
    /* TODO(jec)
	switch (id) {
	case IDHELP:
		DefHelpContext.topic = (char*)"/customcmd.htm";
		g_pOrbiter->OpenHelp (&DefHelpContext);
		return TRUE;
	case IDOK:
		RunFunction (hDlg);
		g_pOrbiter->CloseDialog (hDlg);
		return TRUE;
	case IDC_CUSTOMCMD_LIST:
		switch (code) {
		case LBN_DBLCLK:
			RunFunction (hDlg);
			g_pOrbiter->CloseDialog (hDlg);
			return TRUE;
		case LBN_SELCHANGE:
			ShowDescription (hDlg);
			return TRUE;
		}
		break;
	}
    */
	return DialogWin::OnCommand (hDlg, id, code, hControl);
}
