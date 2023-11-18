// Copyright (c) Martin Schweiger
// Licensed under the MIT License

//=============================================================================
// ScenarioTab class
//=============================================================================

#include <thread>
#include <filesystem>
#include <cstddef>

#include <windows.h>
/* TODO(jec)
#include <io.h>
#include <direct.h>
*/
#include <string>
#include "Orbiter.h"
#include "TabScenario.h"
#include "Launchpad.h"
//#include "Log.h"
#include "Help.h"
#include "htmlctrl.h"
#include "resource.h"
#include "Util.h"

using namespace std;

extern const char* CurrentScenario;
const char *htmlstyle = "<style type=""text/css"">body{font-family:Arial;font-size:12px} p{margin-top:0;margin-bottom:0.5em} h1{font-size:150%;font-weight:normal;margin-bottom:0.5em;color:#000080;background-color:#E6E6FF;padding:0.1em}</style>";

//-----------------------------------------------------------------------------

orbiter::ScenarioTab::ScenarioTab (const LaunchpadDialog *lp): LaunchpadTab (lp)
{
    /* TODO(jec)
	imglist = ImageList_Create (16, 16, ILC_COLOR8, 4, 0);
	treeicon_idx[0] = ImageList_Add (imglist, LoadBitmap (AppInstance(), MAKEINTRESOURCE (IDB_TREEICON_FOLDER1)), 0);
	treeicon_idx[1] = ImageList_Add (imglist, LoadBitmap (AppInstance(), MAKEINTRESOURCE (IDB_TREEICON_FOLDER2)), 0);
	treeicon_idx[2] = ImageList_Add (imglist, LoadBitmap (AppInstance(), MAKEINTRESOURCE (IDB_TREEICON_SCN1)), 0);
	treeicon_idx[3] = ImageList_Add (imglist, LoadBitmap (AppInstance(), MAKEINTRESOURCE (IDB_TREEICON_SCN2)), 0);
    */
	scnhelp[0] = '\0';
	htmldesc = pLp->App()->UseHtmlInline();
}

//-----------------------------------------------------------------------------

orbiter::ScenarioTab::~ScenarioTab ()
{
    /* TODO(jec)
	ImageList_Destroy (imglist);
    */
    if (hThread.joinable()) {
        hThread.join();
    }
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::Create ()
{
    /* TODO(jec)
	hTab = CreateTab (IDD_PAGE_SCN);
    */

	RefreshList(false);
    /* TODO(jec)
	SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_SETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)imglist);

	r_list0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_LIST)); // REMOVE!
	r_desc0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_HTML)); // REMOVE!
	r_pane  = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_SPLIT1));
	r_save0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_SAVE));
	r_clear0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_DELQS));
	r_info0  = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_INFO));
	r_pause0 = GetClientPos (hTab, GetDlgItem (hTab, IDC_SCN_PAUSED));

	if (pLp->App()->UseHtmlInline()) {
		ShowWindow (GetDlgItem (hTab, IDC_SCN_DESC), SW_HIDE);
		ShowWindow (GetDlgItem (hTab, IDC_SCN_HTML), SW_SHOW);
		ShowWindow (GetDlgItem (hTab, IDC_SCN_INFO), SW_HIDE);
		infoId = IDC_SCN_HTML;
	} else {
		ShowWindow (GetDlgItem (hTab, IDC_SCN_HTML), SW_HIDE);
		ShowWindow (GetDlgItem (hTab, IDC_SCN_DESC), SW_SHOW);
		ShowWindow (GetDlgItem (hTab, IDC_SCN_INFO), SW_SHOW);
		infoId = IDC_SCN_DESC;
	}

	splitListDesc.SetHwnd (GetDlgItem (hTab, IDC_SCN_SPLIT1), GetDlgItem (hTab, IDC_SCN_LIST), GetDlgItem (hTab, infoId));
    */

	// create a thread to monitor changes to the scenario list
    hThread = std::thread{threadWatchScnList, this};
	/* hThread = CreateThread (NULL, NULL, threadWatchScnList, this, NULL, NULL); */
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::GetConfig (const Config *cfg)
{
    /* TODO(jec)
	SendDlgItemMessage (hTab, IDC_SCN_PAUSED, BM_SETCHECK,
		cfg->CfgLogicPrm.bStartPaused ? BST_CHECKED : BST_UNCHECKED, 0);
    */
	int listw = cfg->CfgWindowPos.LaunchpadScnListWidth;
	if (!listw) {
		RECT r;
        /* TODO(jec)
		GetClientRect (GetDlgItem (hTab, IDC_SCN_LIST), &r);
        */
		listw = r.right-r.left;
	}
	splitListDesc.SetStaticPane (SplitterCtrl::PANE1, listw);
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::SetConfig (Config *cfg)
{
    /* TODO(jec)
	cfg->CfgLogicPrm.bStartPaused = (SendDlgItemMessage (hTab, IDC_SCN_PAUSED, BM_GETCHECK, 0, 0) == BST_CHECKED);
    */
	cfg->CfgWindowPos.LaunchpadScnListWidth = splitListDesc.GetPaneWidth (SplitterCtrl::PANE1);
}

//-----------------------------------------------------------------------------

bool orbiter::ScenarioTab::OpenHelp ()
{
    /* TODO(jec)
	OpenTabHelp ("tab_scenario");
    */
	return true;
}

//-----------------------------------------------------------------------------

BOOL orbiter::ScenarioTab::OnSize (int w, int h)
{
	int dw = w - (int)(pos0.right-pos0.left);
	int dh = h - (int)(pos0.bottom-pos0.top);
	int w0 = r_pane.right - r_pane.left; // initial splitter pane width
	int h0 = r_pane.bottom - r_pane.top; // initial splitter pane height

	// the elements below may need updating
	int wl0 = r_list0.right - r_list0.left; // initial list width
	int wd0 = r_desc0.right - r_desc0.left; // initial description width
	int wg  = r_desc0.right - r_list0.left - wl0 - wd0;  // gap width
	int bg  = r_clear0.left - r_save0.right; // button gap
	int wb1 = r_save0.right - r_save0.left;
	int wb2 = r_clear0.right - r_clear0.left;
	int wb3 = r_info0.right - r_info0.left;
	int hb  = r_save0.bottom - r_save0.top;
	int wl  = wl0 + (dw*wl0)/(wl0+wd0);
	wl = max (wl, wl0/2);
	int xr = r_list0.left+wl+wg;
	int wr = max(10,wl0+wd0+dw-wl);
	int ww = wl+wr+wg-2*bg;
	wb3 = min (wb3, ww/3);
	ww -= wb3;
	wb1 = wb2 = min (wb1, ww/2);
	int xb2 = r_save0.left+wb1+bg;
	int xb3 = xr+wr-wb3;

    /* TODO(jec)
	SetWindowPos (GetDlgItem (hTab, IDC_SCN_SPLIT1), NULL,
		0, 0, w0+dw, h0+dh,
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos (GetDlgItem (hTab, IDC_SCN_SAVE), NULL,
		r_save0.left, r_save0.top+dh, wb1, hb,
		SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos (GetDlgItem (hTab, IDC_SCN_DELQS), NULL,
		xb2, r_clear0.top+dh, wb2, hb,
		SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos (GetDlgItem (hTab, IDC_SCN_INFO), NULL,
		xb3, r_info0.top+dh, wb3, hb,
		SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos (GetDlgItem (hTab, IDC_SCN_PAUSED), NULL,
		r_pause0.left+dw, r_pause0.top, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOCOPYBITS);
    */

	return NULL;
}

//-----------------------------------------------------------------------------

BOOL orbiter::ScenarioTab::OnNotify(HWND hDlg, int idCtrl, LPNMHDR pnmh)
{
    /* TODO(jec)
	if (idCtrl == IDC_SCN_LIST) {
		NM_TREEVIEW* pnmtv = (NM_TREEVIEW FAR*)pnmh;
		switch (pnmtv->hdr.code) {
		case TVN_SELCHANGED:
			ScenarioChanged();
			return TRUE;
		case NM_DBLCLK:
			PostMessage(LaunchpadWnd(), WM_COMMAND, IDLAUNCH, 0);
			return TRUE;
		}
	}
    */
	return FALSE;
}

//-----------------------------------------------------------------------------

BOOL orbiter::ScenarioTab::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    /* TODO(jec)
	NM_TREEVIEW *pnmtv;

	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SCN_SAVE:
			SaveCurScenario();
			return TRUE;
		case IDC_SCN_DELQS:
			ClearQSFolder();
			return TRUE;
		case IDC_SCN_INFO:
			OpenScenarioHelp();
			return TRUE;
		}
		break;
	}
    */
	return FALSE;
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::RefreshList (bool preserveSelection)
{
	if (Launchpad()->Visible()) {
		char cbuf[256], ch[256], * pc, * c;
		GetSelScenario(cbuf, 256);
        /* TODO(jec)
		SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_SELECTITEM, TVGN_CARET, NULL);
		// remove selection to avoid repeated TVN_SELCHANGED messages while the list is cleared
		//DWORD styles = GetWindowLongPtr(GetDlgItem(hTab, IDC_SCN_LIST), GWL_STYLE);
		SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
		//SetWindowLongPtr(GetDlgItem(hTab, IDC_SCN_LIST), GWL_STYLE, styles);
		ScanDirectory(pCfg->CfgDirPrm.ScnDir, NULL);

		HTREEITEM hti = TreeView_GetRoot(GetDlgItem(hTab, IDC_SCN_LIST));
        */
		if (preserveSelection) { // find the previous selection in the newly created list and re-select it
			pc = cbuf;
			while (*pc) {
				for (c = pc; *c && *c != '\\'; c++);
				bool isdir = (*c == '\\');
				*c = '\0';
                /* TODO(jec)
				TV_ITEM tvi = { TVIF_HANDLE | TVIF_TEXT, 0, 0, 0, ch, 256 };
				for (tvi.hItem = hti; tvi.hItem; tvi.hItem = (HTREEITEM)SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)tvi.hItem)) {
					SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_GETITEM, 0, (LPARAM)&tvi);
					if (!strcmp(tvi.pszText, pc)) {
						hti = tvi.hItem;
						if (isdir)
							hti = (HTREEITEM)SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hti);
						break;
					}
				}
                */
				pc = c;
				if (isdir) pc++;
			}
		}
		else { // Select the "current" scenario
            /* TODO(jec)
			TV_ITEM tvi = { TVIF_HANDLE | TVIF_TEXT, 0, 0, 0, ch, 256 };
			for (tvi.hItem = hti; tvi.hItem; tvi.hItem = (HTREEITEM)SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)tvi.hItem)) {
				SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_GETITEM, 0, (LPARAM)&tvi);
				if (!strcmp(tvi.pszText, CurrentScenario)) {
					hti = tvi.hItem;
					break;
				}
			}
            */
		}
        /* TODO(jec)
		SendDlgItemMessage(hTab, IDC_SCN_LIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hti);
        */
	}
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::LaunchpadShowing(bool show)
{
	if (show) {
		RefreshList(false);
	}
}
//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::ScanDirectory (const char *ppath, HTREEITEM hti)
{
    /* TODO(jec)
	TV_INSERTSTRUCT tvis;
	HTREEITEM ht, hts0, ht0;
    */
	char cbuf[256];

    std::filesystem::path path_ {ppath};

    /* TODO(jec)
	tvis.hParent = hti;
	tvis.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvis.item.pszText = cbuf;
    */

	// scan for subdirectories
    bool first_entry = true;
    for (auto& dir_entry : std::filesystem::recursive_directory_iterator{path_}) {
        const auto& found_path = dir_entry.path();

        if (first_entry) {
            /* TODO(jec)
            tvis.hInsertAfter = TVI_SORT;
            tvis.item.cChildren = 1;
            tvis.item.iImage = treeicon_idx[0];
            tvis.item.iSelectedImage = treeicon_idx[0];
            */
            first_entry = false;
        }

        auto&& found_fname = found_path.filename().string();

        if (dir_entry.is_directory() &&
            !found_fname.empty() &&
            (found_fname.front()!= '.')) {

            strcpy (cbuf, found_fname.c_str());
            /* TODO(jec)
            ht = (HTREEITEM)SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_INSERTITEM, 0, (LPARAM)&tvis);
            */
            /* strcpy (fname, found_fname.c_str()); strcat (fname, "/"); */
            /* TODO(jec):  Fix recursion using recursvie_directory_iterator and
             * treeview.
            ScanDirectory (path_, ht);
            */
        }
        /* TODO(jec)
        hts0 = (HTREEITEM)SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hti);
        */
        // the first subdirectory entry in this folder

        // scan for files
        bool first_scn = true;
        if (found_path.extension() == "scn") {
            if (first_scn) {
                /* TODO(jec)
                tvis.hInsertAfter = TVI_FIRST;
                tvis.item.cChildren = 0;
                tvis.item.iImage = treeicon_idx[2];
                tvis.item.iSelectedImage = treeicon_idx[3];
                */
                first_scn = false;
            }

            strcpy (cbuf, found_fname.c_str());
            cbuf[strlen(cbuf)-4] = '\0';

            char ch[256];
            /* TODO(jec)
            TV_ITEM tvi = {TVIF_HANDLE | TVIF_TEXT, 0, 0, 0, ch, 256};

            ht0 = (HTREEITEM)SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hti);
            for (tvi.hItem = ht0; tvi.hItem && tvi.hItem != hts0; tvi.hItem = (HTREEITEM)SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)tvi.hItem)) {
                SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_GETITEM, 0, (LPARAM)&tvi);
                if (strcmp (tvi.pszText, cbuf) > 0) break;
            }
            if (tvi.hItem) {
                ht = (HTREEITEM)SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_GETNEXTITEM, TVGN_PREVIOUS, (LPARAM)tvi.hItem);
                tvis.hInsertAfter = (ht ? ht : TVI_FIRST);
            } else {
                tvis.hInsertAfter = (hts0 ? TVI_FIRST : TVI_LAST);
            }
            (HTREEITEM)SendDlgItemMessage (hTab, IDC_SCN_LIST, TVM_INSERTITEM, 0, (LPARAM)&tvis);
            */
        }
    }
}

//-----------------------------------------------------------------------------

char *ScanFileDesc (std::istream &is, const char *blockname)
{
	char *buf = 0;
	char blockbegin[256] = "BEGIN_";
	char blockend[256] = "END_";
	strncpy (blockbegin+6, blockname, 240);
	strncpy (blockend+4, blockname, 240);

	if (FindLine (is, blockbegin)) {
		int i, len, buflen = 0;
		const int linelen = 256;
		char line[linelen];
		for(i = 0;; i++) {
			if (!is.getline(line, linelen-2)) {
				if (is.eof()) break;
				else is.clear();
			}
			if (!caseInsensitiveStartsWith(line, blockend)) {
				len = strlen(line);
				if (len) strcat (line, " "), len++;    // convert newline to space
				else     strcpy (line, "\r\n"), len=2; // convert empty line to CR
				char *tmp = new char[buflen+len+1];
				if (buflen) {
					memcpy (tmp, buf, buflen*sizeof(char));
					delete []buf;
				}
				memcpy (tmp+buflen, line, len*sizeof(char));
				buflen += len;
				tmp[buflen] = '\0';
				buf = tmp;
			} else {
				break;
			}
		}
	}
	return buf;
}

void AppendChar (char *&line, int &linelen, char c, int pos)
{
	if (pos == linelen) {
		char *tmp = new char[linelen+256];
		memcpy (tmp, line, linelen);
		delete []line;
		line = tmp;
		linelen += 256;
	}
	line[pos] = c;
}

struct ReplacementPair {
	char *src, *tgt;
};

void Html2Text(std::string& str)
{
	std::string::size_type n0, n1;

	// 1. remove all newlines
	for (int i = str.size() - 1; i >= 0; i--)
		if (str[i] == '\r')
			str.erase(i, 1);
	for (int i = str.size() - 1; i >= 0; i--)
		if (str[i] == '\n')
			str[i] = ' ';

	// 2. substitute some html tags
	const std::string tag[4] = { "</h1> ", "</p> ", "</h1>", "</p>" };
	const std::string tag_subst[4] = { "\r\n\r\n", "\r\n\r\n", "\r\n\r\n", "\r\n\r\n" };
	for (int i = 0; i < 4; i++) {
		n0 = 0;
		while ((n0 = str.find(tag[i], n0)) != std::string::npos) {
			str.replace(n0, tag[i].size(), tag_subst[i]);
			n0 += tag_subst[i].size();
		}
	}

	// 3. remove remaining tags
	n0 = 0;
	while ((n0 = str.find("<", n0)) != std::string::npos) {
		n1 = str.find(">", n0);
		if (n1 != std::string::npos)
			str.erase(n0, n1 - n0 + 1);
	}

	// 4. substitute some symbols
	const std::string sym[5] = { "&gt;", "&lt;", "&ge;", "&le;", "&amp;" };
	const std::string sym_subst[5] = { ">", "<", ">=", "<=", "\001" };
	for (int i = 0; i < 5; i++) {
		n0 = 0;
		while ((n0 = str.find(sym[i], n0)) != std::string::npos) {
			str.replace(n0, sym[i].size(), sym_subst[i]);
			n0 += sym_subst[i].size();
		}
	}

	// 5. remove remaining symbols
	n0 = 0;
	while ((n0 = str.find("&", n0)) != std::string::npos) {
		n1 = str.find(";", n0);
		if (n1 != std::string::npos)
			str.erase(n0, n1 - n0 + 1);
	}

	// 6. restore ampersands
	for (std::size_t i = 0; i < str.size(); i++)
		if (str[i] == '\001')
			str[i] = '&';
}

void Text2Html(std::string& str)
{
	std::string::size_type n0;

	// 1. substitute some symbols
	const std::string sym[6] = { "&", ">=", "<=", ">", "<", "\r\n" }; // the order is relevant here
	const std::string sym_subst[6] = { "&amp;", "&ge;", "&le;", "&gt;", "&lt;", "<br />"};
	for (int i = 0; i < 6; i++) {
		n0 = 0;
		while ((n0 = str.find(sym[i], n0)) != std::string::npos) {
			str.replace(n0, sym[i].size(), sym_subst[i]);
			n0 += sym_subst[i].size();
		}
	}
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::ScenarioChanged ()
{
	const int linelen = 256;
	bool have_info = false;
	char cbuf[256], *pc;
	ifstream ifs;
	scnhelp[0] = '\0';

	switch (GetSelScenario (cbuf, 256)) {
	case 0: // error
		return;
	case 1: // scenario file
		ifs.open (pLp->App()->ScnPath (cbuf));
		pLp->EnableLaunchButton (true);
		break;
	case 2: { // subdirectory
        auto path = std::filesystem::path{pCfg->CfgDirPrm.ScnDir} /
                    cbuf /
                    "Description.txt";
		ifs.open (path.c_str(), ios::in);
		pLp->EnableLaunchButton (false);
		break;
    }
	}

	if (ifs) {
		if (!have_info) {
			char *buf;
			if (htmldesc) {
				buf = ScanFileDesc(ifs, "URLDESC");
				if (buf) {
					char url_ref[256], url[256], *path, *topic;
					strncpy(url_ref, trim_string(buf), 255);
					path = strtok(url_ref, ",");
					topic = strtok(NULL, "\n");
					if (topic) {
						sprintf(url, "its:Html\\Scenarios\\%s.chm::%s.htm", path, topic);
                    } else {
						sprintf(url, "%s\\Html\\Scenarios\\%s.htm",
                                std::filesystem::current_path().c_str(),
                                path);
                    }
                    /* TODO(jec)
					DisplayHTMLPage(GetDlgItem(hTab, IDC_SCN_HTML), url);
                    */
					have_info = true;
				}
				else {
					buf = ScanFileDesc(ifs, "HYPERDESC");
					if (!buf) {
						buf = ScanFileDesc(ifs, "DESC");
						if (buf) {
							std::string str(buf);
							Text2Html(str);
							delete[]buf;
							buf = new char[str.size() + 1];
							strcpy(buf, str.c_str());
						}
					}
					if (buf) { // prepend style preamble
						char* buf2 = new char[strlen(htmlstyle) + strlen(buf) + 1];
						strcpy(buf2, htmlstyle); strcat(buf2, buf);
						delete[]buf;
						buf = buf2;
                        /* TODO(jec)
						DisplayHTMLStr(GetDlgItem(hTab, IDC_SCN_HTML), buf);
                        */
						have_info = true;
					}
				}
			} else {
				if (buf = ScanFileDesc (ifs, "DESC")) {
                    /* TODO(jec)
					SetWindowText(GetDlgItem(hTab, IDC_SCN_DESC), buf);
                    */
					have_info = true;
				} else if (buf = ScanFileDesc (ifs, "HYPERDESC")) {
					std::string str(buf);
					Html2Text(str);
                    /* TODO(jec)
					SetWindowText(GetDlgItem(hTab, IDC_SCN_DESC), str.c_str());
                    */
					have_info = true;
				}
			}
			if (buf) {
				delete []buf;
				buf = NULL;
			}
		}
	}

	if (!have_info) {
        /* TODO(jec)
		if (htmldesc) DisplayHTMLStr (GetDlgItem (hTab, IDC_SCN_HTML), "");
		else          SetWindowText (GetDlgItem (hTab, IDC_SCN_DESC), "");
        */
	}

	if (!htmldesc) {
		bool enable_info = false;
		for (int i = 0; scnhelp[i]; i++) {
			if (scnhelp[i] == ',') {
				enable_info = true;
				break;
			}
        }
        /* TODO(jec)
		EnableWindow (GetDlgItem (hTab, IDC_SCN_INFO), enable_info ? TRUE:FALSE);
        */
	}
}

//-----------------------------------------------------------------------------

int orbiter::ScenarioTab::GetSelScenario (char *scn, int len)
{
    /* TODO(jec)
	TV_ITEM tvi;
    */
	char cbuf[256];
	int type;

    /* TODO(jec)
	tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_CHILDREN;
	tvi.hItem = TreeView_GetSelection (GetDlgItem (hTab, IDC_SCN_LIST));
	tvi.pszText = scn;
	tvi.cchTextMax = len;

	if (!TreeView_GetItem (GetDlgItem (hTab, IDC_SCN_LIST), &tvi)) return 0;
	type = (tvi.cChildren ? 2 : 1);

	// build path
	tvi.pszText = cbuf;
	tvi.cchTextMax = 256;
	while (tvi.hItem = TreeView_GetParent (GetDlgItem (hTab, IDC_SCN_LIST), tvi.hItem)) {
		if (TreeView_GetItem (GetDlgItem (hTab, IDC_SCN_LIST), &tvi)) {
			strcat (cbuf, "\\");
			strcat (cbuf, scn);
			strcpy (scn, cbuf);
		}
	}
    */
	return type;
}

//-----------------------------------------------------------------------------

void orbiter::ScenarioTab::SaveCurScenario ()
{
	ifstream ifs (pLp->App()->ScnPath (CurrentScenario), ios::in);
	if (ifs) {
        /* TODO(jec)
		DialogBoxParam (AppInstance(), MAKEINTRESOURCE(IDD_SAVESCN), LaunchpadWnd(), SaveProc, (LPARAM)this);
        */
	} else {
        /* TODO(jec)
		MessageBox (LaunchpadWnd(), "No current simulation state available", "Save Error", MB_OK|MB_ICONEXCLAMATION);
        */
	}
}

//-----------------------------------------------------------------------------
// Name: SaveCurScenarioAs()
// Desc: copy current scenario file into 'name', replacing description with 'desc'.
//		 return value: 0=ok, 1=failed, 2=file exists (only checked if replace=false)
//-----------------------------------------------------------------------------
int orbiter::ScenarioTab::SaveCurScenarioAs (const char *name, char *desc, bool replace)
{
	string cbuf;
	bool skip = false;
	const char *path = pLp->App()->ScnPath (name);
	if (!replace) { // check if exists
		ifstream ifs (path, ios::in);
		if (ifs) return 2;
	}
	ofstream ofs (path);
	if (!ofs) return 1;
	ifstream ifs (pLp->App()->ScnPath (CurrentScenario));
	if (!ifs) return 1;
	int i, len = strlen(desc);
	for (i = 0; i < len-1; i++)
		if (desc[i] == '\r' && desc[i+1] == '\n') desc[i] = '\n';
	ofs << "BEGIN_DESC" << endl;
	ofs << desc << endl;
	ofs << "END_DESC" << endl;
	while (std::getline( ifs, cbuf ))
	{
		if (cbuf == "BEGIN_DESC")
			skip = true;
		else if (cbuf == "END_DESC")
			skip = false;
		else if (!skip)
			ofs << cbuf << endl;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Name: SaveProc()
// Desc: Scenario save dialog message proc
//-----------------------------------------------------------------------------
INT_PTR CALLBACK orbiter::ScenarioTab::SaveProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ScenarioTab *pTab;
	int res, name_len, desc_len;
	static char name[64], *desc;

    /* TODO(jec)
	switch (uMsg) {
	case WM_INITDIALOG:
		pTab = (ScenarioTab*)lParam;
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			name_len = SendDlgItemMessage (hWnd, IDC_SAVE_NAME, WM_GETTEXTLENGTH, 0, 0);
			desc_len = SendDlgItemMessage (hWnd, IDC_SAVE_DESC, WM_GETTEXTLENGTH, 0, 0);
			if (name_len > 63) {
				MessageBox (hWnd, "Scenario name too long (max 63 characters)", "Save Error", MB_OK|MB_ICONEXCLAMATION);
				return TRUE;
			}
			desc = new char[desc_len+1];
			SendDlgItemMessage (hWnd, IDC_SAVE_NAME, WM_GETTEXT, 64, (LPARAM)name);
			SendDlgItemMessage (hWnd, IDC_SAVE_DESC, WM_GETTEXT, desc_len+1, (LPARAM)desc);
			res = pTab->SaveCurScenarioAs (name, desc);
			if (res == 2) {
				if (MessageBox (hWnd, "File exists. Overwrite?", "Warning", MB_YESNO|MB_ICONQUESTION) == IDYES)
					res = pTab->SaveCurScenarioAs (name, desc, true);
				else return TRUE;
			}
			if (res == 1) {
				MessageBox (hWnd, "Error writing scenario file.", "Save Error", MB_OK|MB_ICONEXCLAMATION);
				return TRUE;
			}
			delete []desc;
			desc = NULL;
			// fall through
		case IDCANCEL:
			EndDialog (hWnd, TRUE);
			return TRUE;
		}
	}
    */
    return FALSE;
}

//-----------------------------------------------------------------------------
// Name: ClearQSFolder()
// Desc: Delete all scenarios in the Quicksave folder
//-----------------------------------------------------------------------------
void orbiter::ScenarioTab::ClearQSFolder()
{
	char filespec[256];
	strcpy (filespec, pLp->App()->ScnPath ("Quicksave\\*"));
    // At this point it's "<some_path>/Quicksave/*.scn"
    std::filesystem::path quicksavePath {filespec};
    quicksavePath.remove_filename();
    for (auto& dir_entry : std::filesystem::directory_iterator{quicksavePath}) {
        if (dir_entry.path().extension() == "scn") {
            std::filesystem::remove(dir_entry.path());
        }
    }
}

//-----------------------------------------------------------------------------
// Name: OpenScenarioHelp()
// Desc: Opens the help file associated with the scenario
//-----------------------------------------------------------------------------
void orbiter::ScenarioTab::OpenScenarioHelp ()
{
	if (!scnhelp[0]) return;
	char str[256], *scenario, *topic;
	strncpy (str, scnhelp, 256);
	scenario = strtok (str, ",");
	topic = strtok (NULL, "\n");
    auto path = std::filesystem::path{"html"} / "scenarios" / scenario;
    path += ".chm";
	::OpenHelp(LaunchpadWnd(), path.c_str(), topic);
}

//-----------------------------------------------------------------------------
// Thread function for scenario directory tree watcher
//-----------------------------------------------------------------------------
void orbiter::ScenarioTab::threadWatchScnList (LPVOID pPrm)
{
	ScenarioTab *tab = (ScenarioTab*)pPrm;
    /* TODO:  This function gets event notifications from the OS on changes in
     * the scenarios folder, asynchronously.  std::filesystem doesn't have
     * anything like that.
	HANDLE dwChangeHandle;
	DWORD dwWaitStatus;

	dwChangeHandle = FindFirstChangeNotification (
		tab->pCfg->CfgDirPrm.ScnDir,
		TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);

	while (true) {
		dwWaitStatus = WaitForSingleObject (dwChangeHandle, INFINITE);
		switch (dwWaitStatus) {
			case WAIT_OBJECT_0:
				tab->RefreshList(true);
				FindNextChangeNotification (dwChangeHandle);
				break;
		}
	}
	FindCloseChangeNotification(dwChangeHandle);
    */
}
