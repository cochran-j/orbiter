// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#define STRICT 1
#define ORBITER_MODULE
#include <filesystem>
#include "Orbitersdk.h"
#include "AC_resource.h"
#include <stdio.h>
/* TODO(jec)
#include <io.h>
*/

class VesselConfig;
class AtlantisConfig;

using std::filesystem::path;

static const path tex_hires_enabled =
    path{"Textures2"} / "Atlantis";
static const path tex_hires_disabled = path{"Textures2"} / "~Atlantis";

static const path vcmsh_fname = path{"Meshes"} / "Atlantis" / "AtlantisVC.msh";
static const path msh_hires_bkup = path{"Meshes"} / "Atlantis" / "~AtlantisVC_hi.msh";
static const path msh_lores_bkup = path{"Meshes"} / "Atlantis" / "~AtlantisVC_lo.msh";

struct {
	HINSTANCE hInst;
	AtlantisConfig *item;
} gParams;

class AtlantisConfig: public LaunchpadItem {
public:
	AtlantisConfig(): LaunchpadItem() {}
	char *Name() { return (char*)"Atlantis Configuration"; }
	char *Description();
	bool clbkOpen (HWND hLaunchpad);
	bool TexHiresEnabled() const;
	void TexEnableHires (bool enable);
	bool MshHiresEnabled() const;
	bool MshEnableHires (bool enable);
	void InitDialog (HWND hWnd);
	void Apply (HWND hWnd);
	static INT_PTR DlgProc (HWND, UINT, WPARAM, LPARAM);
};

char *AtlantisConfig::Description()
{
	return (char*)"Global configuration for the default Space Shuttle Atlantis.";
}

bool AtlantisConfig::clbkOpen (HWND hLaunchpad)
{
	// respond to user double-clicking the item in the list
	return OpenDialog (gParams.hInst, hLaunchpad, IDD_ACONFIG, DlgProc);
}

bool AtlantisConfig::TexHiresEnabled () const
{
	// check if the Atlantis highres texture directory is present
	return std::filesystem::exists(tex_hires_enabled);
}

void AtlantisConfig::TexEnableHires (bool enable)
{
	if (TexHiresEnabled() == enable) return; // nothing to do

	if (enable) {
        std::filesystem::rename (tex_hires_disabled, tex_hires_enabled);
	} else {
		// to disable the highres textures, we simply rename the directory
		// so that orbiter's texture manager can't find it
        std::filesystem::rename (tex_hires_enabled, tex_hires_disabled);
	}
}

bool AtlantisConfig::MshHiresEnabled () const
{
	// check if backup of low-res mesh is present
	return (std::filesystem::exists(msh_lores_bkup));
}

bool AtlantisConfig::MshEnableHires (bool enable)
{
	if (MshHiresEnabled() == enable) return false; // nothing to do

	if (enable) {
		if (!std::filesystem::exists(msh_hires_bkup)) return false; // high-res backup not found
		if (std::filesystem::exists(vcmsh_fname) &&
            !std::filesystem::exists(msh_lores_bkup)) {
			std::filesystem::rename (vcmsh_fname, msh_lores_bkup); // back up low-res mesh
        }
        std::filesystem::rename (msh_hires_bkup, vcmsh_fname); // activate high-res mesh
	} else {
		if (!std::filesystem::exists(msh_lores_bkup)) return false; // low-res backup not found
		if (std::filesystem::exists(vcmsh_fname) &&
            !std::filesystem::exists(msh_hires_bkup)) {
            std::filesystem::rename (vcmsh_fname, msh_hires_bkup); // back up high-res mesh
        }
        std::filesystem::rename (msh_lores_bkup, vcmsh_fname); // activate low-res mesh
	}
	return true;
}

void AtlantisConfig::InitDialog (HWND hWnd)
{
	bool texhires = TexHiresEnabled();
	bool mshhires = MshHiresEnabled();
    /* TODO(jec)
	SendDlgItemMessage (hWnd, IDC_RADIO1, BM_SETCHECK, texhires?BST_CHECKED:BST_UNCHECKED, 0);
	SendDlgItemMessage (hWnd, IDC_RADIO2, BM_SETCHECK, texhires?BST_UNCHECKED:BST_CHECKED, 0);
	SendDlgItemMessage (hWnd, IDC_RADIO3, BM_SETCHECK, mshhires?BST_CHECKED:BST_UNCHECKED, 0);
	SendDlgItemMessage (hWnd, IDC_RADIO4, BM_SETCHECK, mshhires?BST_UNCHECKED:BST_CHECKED, 0);
    */
}

void AtlantisConfig::Apply (HWND hWnd)
{
    /* TODO(jec)
	bool texhires = (SendDlgItemMessage (hWnd, IDC_RADIO1, BM_GETCHECK, 0, 0) == BST_CHECKED);
	TexEnableHires (texhires);
	bool mshhires = (SendDlgItemMessage (hWnd, IDC_RADIO3, BM_GETCHECK, 0, 0) == BST_CHECKED);
	MshEnableHires (mshhires);
    */
}

INT_PTR AtlantisConfig::DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    /* TODO(jec)
	switch (uMsg) {
	case WM_INITDIALOG:
		((AtlantisConfig*)lParam)->InitDialog (hWnd);
		break;
	case WM_COMMAND:
		switch (LOWORD (wParam)) {
		case IDOK:
			((AtlantisConfig*)GetWindowLongPtr (hWnd, DWLP_USER))->Apply (hWnd);
			EndDialog (hWnd, 0);
			return 0;
		case IDCANCEL:
			EndDialog (hWnd, 0);
			return 0;
		}
		break;
	}
    */
	return 0;
}

// ==============================================================
// The DLL entry point
// ==============================================================

DLLCLBK void InitModule (HINSTANCE hDLL)
{
	gParams.hInst = hDLL;
	gParams.item = new AtlantisConfig;
	// create the new config item
	LAUNCHPADITEM_HANDLE root = oapiFindLaunchpadItem ("Vessel configuration");
	// find the config root entry provided by orbiter
	oapiRegisterLaunchpadItem (gParams.item, root);
	// register the DG config entry
}

// ==============================================================
// The DLL exit point
// ==============================================================

DLLCLBK void ExitModule (HINSTANCE hDLL)
{
	// Unregister the launchpad items
	oapiUnregisterLaunchpadItem (gParams.item);
	delete gParams.item;
}
