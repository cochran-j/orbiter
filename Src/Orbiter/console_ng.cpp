// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#include "console_ng.h"
#include "Orbiter.h"
#include "DlgMgr.h"
#include "Psys.h"
#include "Vessel.h"
#include "Log.h"
#include "DlgFocus.h"
#include "DlgMap.h"
#include "DlgInfo.h"
#include "DlgTacc.h"
#include "DlgFunction.h"
#include "DlgRecorder.h"
#include "DlgHelp.h"
#include "ConsoleManager.h"
#include "resource.h"
#include "Util.h"

#include <thread>
#include <mutex>
#include <cstdio>

extern PlanetarySystem* g_psys;
extern Vessel* g_focusobj;
extern TimeData td;

static DWORD WINAPI InputProc(LPVOID);
static INT_PTR CALLBACK ServerDlgProc(HWND, UINT, WPARAM, LPARAM);
static void ConsoleOut(const char* msg);

static std::mutex hMutex {};
static HANDLE s_hStdO = NULL;
static char cConsoleCmd[1024] = "\0";
static orbiter::ConsoleNG* s_console = NULL; // access to console instance from message callback functions

orbiter::ConsoleNG::ConsoleNG(Orbiter* pOrbiter)
    : m_pOrbiter(pOrbiter)
    , m_hWnd(NULL)
    , m_hStatWnd(NULL)
    , m_hThread()
{
    static PCSTR title = "Orbiter Server Console";
    static SIZE_T stackSize = 4096;

    s_console = this;

    ConsoleManager::ShowConsole(true);
    DWORD id;
    /* TODO(jec)  Windows console stuff
    SetConsoleTitle(title);
    m_hWnd = GetConsoleWindow();
	if (ConsoleManager::IsConsoleExclusive())
		DeleteMenu(GetSystemMenu(m_hWnd, false), SC_CLOSE, MF_BYCOMMAND);
    m_hThread = CreateThread(NULL, stackSize, InputProc, this, 0, &id);
    */
    m_hThread = std::thread{InputProc, this};
    /*
    s_hStdO = GetStdHandle(STD_OUTPUT_HANDLE);
    */
    SetLogOutFunc(&ConsoleOut); // clone log output to console
}

orbiter::ConsoleNG::~ConsoleNG()
{
	DestroyStatDlg();
	SetLogOutFunc(0);
    m_hThread.join();
	s_console = NULL;
	s_hStdO = NULL;
}

bool orbiter::ConsoleNG::ParseCmd()
{
	if (!cConsoleCmd[0]) return false;
	char cmd[1024], cbuf[256], * pc, * ppc;

    hMutex.lock();
	strcpy(cmd, cConsoleCmd + 1);
	cConsoleCmd[0] = '\0';
    hMutex.unlock();

	DWORD i;
	if (caseInsensitiveStartsWith(cmd, "help")) {
		pc = trim_string(cmd + 4);
		if (caseInsensitiveStartsWith(pc, "help")) {
			Echo("Brief onscreen help for console commands.");
			Echo("Type \"help\" followed by a top-level command to get information for this command.");
		}
		else if (caseInsensitiveStartsWith(pc, "exit")) {
			Echo("Exits the simulation session and returns to the Launchpad dialog.");
		}
		else if (caseInsensitiveStartsWith(pc, "vessel")) {
			ppc = trim_string(pc + 6);
			if (caseInsensitiveStartsWith(ppc, "list")) {
				Echo("Lists all vessels in the current session.");
			}
			else if (caseInsensitiveStartsWith(ppc, "count")) {
				Echo("Prints the number of vessels in the current session.");
			}
			else if (caseInsensitiveStartsWith(ppc, "focus")) {
				Echo("Prints the name of the current focus vessel.");
			}
			else if (caseInsensitiveStartsWith(ppc, "del")) {
				Echo("vessel del <name> -- Destroy vessel <name>.");
			}
			else {
				Echo("Vessel-specific commands. The following sub-commands are recognized:\n");
				Echo("list count focus del\n");
				Echo("Type \"help vessel <subcommand>\" to get information for a command.");
			}
		}
		else if (caseInsensitiveStartsWith(pc, "time")) {
			Echo("Output current simulation time.");
			Echo("time syst  --  Session up time (seconds)");
			Echo("time simt  --  Simulation time (seconds)");
			Echo("time mjd   --  Absolute simulation time (MJD format)");
			Echo("time ut    --  Absolute simulation time (UT format)");
			Echo("Without arguments, all 4 time values are displayed.");
		}
		else if (caseInsensitiveStartsWith(pc, "tacc")) {
			Echo("Display or set time acceleration factor.");
			Echo("tacc <x>  --  Set new time acceleration factor x.");
			Echo("Without argument, prints the current time acceleration factor.");
		}
		else if (caseInsensitiveStartsWith(pc, "pause")) {
			Echo("Pause/resume simulation session.");
			Echo("pause on      --  pause simulation");
			Echo("pause off     --  resume simulation");
			Echo("pause toggle  --  toggle pause/resume state");
			Echo("Without arguments, the current simulation state is displayed.");
		}
		else if (caseInsensitiveStartsWith(pc, "step")) {
			Echo("Display momentary simulation step length and steps per second.");
		}
		else if (caseInsensitiveStartsWith(pc, "dlg")) {
			Echo("Open a dialog.");
			Echo("dlg focus    -- Open the vessel selction dialog");
			Echo("dlg map      -- Open the map window");
			Echo("dlg info     -- Open the object info dialog");
			Echo("dlg tacc     -- Open the time acceleration dialog");
			Echo("dlg help     -- Open the help dialog");
			Echo("dlg record   -- Open the flight recorder dialog");
			Echo("dlg function -- Open the plugin function list");
		}
		else if (caseInsensitiveStartsWith(pc, "gui")) {
			Echo("Toggles the display of a dialog box that continuously monitors the simulation");
			Echo("state.");
		}
		else {
			Echo("The following top-level commands are available:\n");
			Echo("  help exit vessel time tacc pause step dlg gui\n");
			Echo("To get help for a command, type \"help <cmd>\"");
		}
	}
	else if (caseInsensitiveStartsWith(cmd, "exit")) {
		m_pOrbiter->CloseSession();
		return true;
	}
	else if (caseInsensitiveStartsWith(cmd, "vessel")) {
		pc = trim_string(cmd + 6);
		if (caseInsensitiveStartsWith(pc, "list")) {
			for (i = 0; i < g_psys->nVessel(); i++)
				Echo(g_psys->GetVessel(i)->Name());
			return true;
		}
		else if (caseInsensitiveStartsWith(pc, "count")) {
			sprintf(cbuf, "%zu", g_psys->nVessel());
			Echo(cbuf);
		}
		else if (caseInsensitiveStartsWith(pc, "focus")) {
			Echo(g_focusobj->Name());
		}
		else if (caseInsensitiveStartsWith(pc, "del")) {
			Vessel* v = g_psys->GetVessel(trim_string(pc + 3), true);
			if (v) v->RequestDestruct();
		}
	}
	else if (caseInsensitiveStartsWith(cmd, "tacc")) {
		double w;
		if (sscanf(trim_string(cmd + 4), "%lf", &w) == 1)
			m_pOrbiter->SetWarpFactor(w);
		else {
			sprintf(cbuf, "Time acceleration is %0.1f", td.Warp());
			Echo(cbuf);
		}
	}
	else if (caseInsensitiveStartsWith(cmd, "time")) {
		pc = trim_string(cmd + 4);
		if (caseInsensitiveStartsWith(pc, "simt")) {
			sprintf(cbuf, "%0.1f", td.SimT0);
		}
		else if (caseInsensitiveStartsWith(pc, "syst")) {
			sprintf(cbuf, "%0.1f", td.SysT0);
		}
		else if (caseInsensitiveStartsWith(pc, "mjd")) {
			sprintf(cbuf, "%0.6f", td.MJD0);
		}
		else if (caseInsensitiveStartsWith(pc, "ut")) {
			strcpy(cbuf, DateStr(td.MJD0));
		}
		else {
			sprintf(cbuf, "SysT=%0.1f SimT=%0.1f, MJD=%0.6f, UT=%s", td.SysT0, td.SimT0, td.MJD0, DateStr(td.MJD0));
		}
		Echo(cbuf);
	}
	else if (caseInsensitiveStartsWith(cmd, "pause")) {
		pc = trim_string(cmd + 5);
		if (caseInsensitiveStartsWith(pc, "on")) m_pOrbiter->Pause(true);
		else if (caseInsensitiveStartsWith(pc, "off")) m_pOrbiter->Pause(false);
		else if (caseInsensitiveStartsWith(pc, "toggle")) m_pOrbiter->TogglePause();
        std::snprintf(cbuf, 256, "Simulation %s", m_pOrbiter->IsRunning() ? "running" : "paused");
		Echo(cbuf);
	}
	else if (caseInsensitiveStartsWith(cmd, "step")) {
        std::snprintf(cbuf, 256, "dt=%f, FPS=%f", td.SimDT, td.FPS());
		Echo(cbuf);
	}
	else if (caseInsensitiveStartsWith(cmd, "gui")) {
        /* TODO(jec)
		if (!DestroyStatDlg())
			m_hStatWnd = CreateDialog(m_pOrbiter->GetInstance(), MAKEINTRESOURCE(IDD_SERVER), m_hWnd, ServerDlgProc);
        */
	}
	else if (caseInsensitiveStartsWith(cmd, "dlg")) {
		DialogManager* pDlgMgr = m_pOrbiter->DlgMgr();
		if (pDlgMgr) {
			pc = trim_string(cmd + 3);
			if (caseInsensitiveStartsWith(pc, "focus"))
				pDlgMgr->EnsureEntry<DlgFocus>();
			else if (caseInsensitiveStartsWith(pc, "map"))
				pDlgMgr->EnsureEntry<DlgMap>();
			else if (caseInsensitiveStartsWith(pc, "info"))
				pDlgMgr->EnsureEntry<DlgInfo>();
			else if (caseInsensitiveStartsWith(pc, "tacc"))
				pDlgMgr->EnsureEntry<DlgTacc>();
			else if (caseInsensitiveStartsWith(pc, "function"))
				pDlgMgr->EnsureEntry<DlgFunction>();
			else if (caseInsensitiveStartsWith(pc, "record"))
				pDlgMgr->EnsureEntry<DlgRecorder>();
			else if (caseInsensitiveStartsWith(pc, "help"))
				pDlgMgr->EnsureEntry<DlgHelp>();
		}
	}
	return false;
}

void orbiter::ConsoleNG::Echo(const char* str) const
{
	ConsoleOut(str);
}

void orbiter::ConsoleNG::EchoIntro() const
{
	Echo("-----------------\nOrbiter NG (no graphics)");
	Echo("Running in server mode (no graphics client attached).");
	Echo("Type \"help\" for a list of commands.");
	Echo("Type \"exit\" to return to the Launchpad dialog.\n");
}

bool orbiter::ConsoleNG::DestroyStatDlg()
{
	if (m_hStatWnd) {
        /* TODO(jec)
		DestroyWindow(m_hStatWnd);
        */
		m_hStatWnd = NULL;
		return true;
	}
	else
		return false;
}


DWORD WINAPI InputProc(LPVOID context)
{
	DWORD count = 0, c;
	char cbuf[1024];
    /* TODO(jec) Windows console stuff
	HANDLE hStdI = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdO = GetStdHandle(STD_OUTPUT_HANDLE);
    */
	orbiter::ConsoleNG* console = (orbiter::ConsoleNG*)context;
    /*
	SetConsoleMode(hStdI, ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
	SetConsoleTextAttribute(hStdI, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
*/

    /* NOTE(jec) std::mutex is successful or throws
	if (!hMutex)
		return 1;
    */
	for (;;) {
        /*
		if (!ReadConsole(hStdI, cbuf, 1024, &count, NULL))
			break; // Console not available, exiting
		WriteConsole(hStdO, "> ", 2, &c, NULL);
*/

        hMutex.lock();
		/* WaitForSingleObject(hMutex, 1000); */
		cConsoleCmd[0] = 'x';
		strncpy(cConsoleCmd + 1, cbuf, count);
        if (count) {
            cConsoleCmd[count - 1] = '\0'; // eliminates CR
        }
        hMutex.unlock();
		/* ReleaseMutex(hMutex); */

		// handle "exit" directly so we can terminate the console thread in an orderly fashion
		if (!strncmp(cbuf, "exit\r\n", 6)) {
			break;
		}
	}
	return 0;
}

INT_PTR CALLBACK ServerDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    /* TODO(jec)
	switch (uMsg) {
	case WM_INITDIALOG:
		SetTimer(hDlg, 1, 1000, NULL);
		return TRUE;
	case WM_TIMER:
		if (s_console)
			s_console->GetOrbiter()->UpdateServerWnd(hDlg);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (s_console)
				s_console->GetOrbiter()->CloseSession();
		}
		break;
	case WM_CLOSE:
		if (s_console)
			s_console->DestroyStatDlg();
		return 0;
	case WM_DESTROY:
		KillTimer(hDlg, 1);
		return 0;
	}
    */
	return FALSE;
}

void ConsoleOut(const char* msg)
{
	if (!s_hStdO) return;
	DWORD count;
    /* TODO(jec)
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SetConsoleTextAttribute(s_hStdO, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	GetConsoleScreenBufferInfo(s_hStdO, &csbi);
	csbi.dwCursorPosition.X = 0;
	SetConsoleCursorPosition(s_hStdO, csbi.dwCursorPosition);
	WriteConsole(s_hStdO, msg, strlen(msg), &count, NULL);
	SetConsoleTextAttribute(s_hStdO, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	WriteConsole(s_hStdO, "\n> ", 3, &count, NULL);
    */
}
