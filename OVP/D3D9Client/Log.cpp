// =================================================================================================================================
// The MIT Lisence:
//
// Copyright (C) 2012 - 2016 Jarmo Nikkanen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
// IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// =================================================================================================================================

#include <windows.h>
#include "Log.h"
#include "D3D9Util.h"
#include "D3D9Config.h"
#include "D3D9Client.h"

#include <mutex>
#include <thread>
#include <chrono>
#include <cstring>

FILE *d3d9client_log = NULL;

#define LOG_MAX_LINES 100000
#define ERRBUF 8000
#define OPRBUF 512
#define TIMEBUF 63

extern class D3D9Client* g_client;

char ErrBuf[ERRBUF+1];
char OprBuf[OPRBUF+1];
char TimeBuf[TIMEBUF+1];

time_t ltime;
int uEnableLog = 1;     // This value is controlling log opeation ( Config->DebugLvl )
int iEnableLog = 0;     // Index into EnableLogStack
int EnableLogStack[16];
int iLine = 0;          // Line number counter (iLine <= LOG_MAX_LINES)

using d3d_clock = std::chrono::high_resolution_clock;
using dp_seconds = std::chrono::duration<double>;
using dp_milliseconds = std::chrono::duration<double, std::ratio<1, 1000>>;
using dp_microseconds = std::chrono::duration<double, std::ratio<1, 1000000>>;

d3d_clock::time_point qpcRef {};     // Performance counter reference value (for "delta t")
d3d_clock::time_point qpcStart {};   // Performance counter start value ("zero")

std::queue<std::string> D3D9DebugQueue;

std::mutex LogCrit{};


//-------------------------------------------------------------------------------------------
//
void MissingRuntimeError()
{
    /* TODO(jec)
	MessageBoxA(NULL,
		"DirectX Runtimes may be missing. See /Doc/D3D9Client.pdf for more information",
		"D3D9Client Initialization Failed", MB_OK);
    */
}

//-------------------------------------------------------------------------------------------
//
void RuntimeError(const char* File, const char* Fnc, UINT Line)
{
	if (Config->DebugLvl == 0) return;
	char buf[256];
    std::snprintf(buf, 256, "[%s] [%s] Line: %u See Orbiter.log for details.", File, Fnc, Line);
    /* TODO(jec)
	MessageBoxA(g_client->GetRenderWindow(), buf, "Critical Error:", MB_OK);
	DebugBreak();
    */
}

//-------------------------------------------------------------------------------------------
//
/*
int PrintModules(DWORD pAdr)
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	unsigned int i;

	// Get a handle to the process.

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetProcessId(GetCurrentProcess()));

	if (NULL == hProcess) return 1;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			char szModName[MAX_PATH];
			if (GetModuleFileNameExA(hProcess, hMods[i], szModName, sizeof(szModName))) {
				MODULEINFO mi;
				GetModuleInformation(hProcess, hMods[i], &mi, sizeof(MODULEINFO));
				DWORD Base = (DWORD)mi.lpBaseOfDll;
				if (pAdr > Base && pAdr < (Base + mi.SizeOfImage)) LogErr("%s EntryPoint=0x%8.8X, Base=0x%8.8X, Size=%u", szModName, mi.EntryPoint, mi.lpBaseOfDll, mi.SizeOfImage);
				else										 LogOk("%s EntryPoint=0x%8.8X, Base=0x%8.8X, Size=%u", szModName, mi.EntryPoint, mi.lpBaseOfDll, mi.SizeOfImage);
			}
		}
	}
	CloseHandle(hProcess);
	return 0;
}
*/


//-------------------------------------------------------------------------------------------
// Log OAPISURFACE_xxx attributes
void LogAttribs(DWORD attrib, DWORD w, DWORD h, LPCSTR origin)
{
	char buf[512];
    std::snprintf(buf, 512, "%s (%d,%d)[0x%X]: ", origin, w, h, attrib);
	if (attrib&OAPISURFACE_TEXTURE)		 std::strncat(buf, "OAPISURFACE_TEXTURE ", 512);
	if (attrib&OAPISURFACE_RENDERTARGET) std::strncat(buf, "OAPISURFACE_RENDERTARGET ", 512);
	if (attrib&OAPISURFACE_GDI)			 std::strncat(buf, "OAPISURFACE_GDI ", 512);
	if (attrib&OAPISURFACE_SKETCHPAD)	 std::strncat(buf, "OAPISURFACE_SKETCHPAD ", 512);
	if (attrib&OAPISURFACE_MIPMAPS)		 std::strncat(buf, "OAPISURFACE_MIPMAPS ", 512);
	if (attrib&OAPISURFACE_NOMIPMAPS)	 std::strncat(buf, "OAPISURFACE_NOMIPMAPS ", 512);
	if (attrib&OAPISURFACE_ALPHA)		 std::strncat(buf, "OAPISURFACE_ALPHA ", 512);
	if (attrib&OAPISURFACE_NOALPHA)		 std::strncat(buf, "OAPISURFACE_NOALPHA ", 512);
	if (attrib&OAPISURFACE_UNCOMPRESS)	 std::strncat(buf, "OAPISURFACE_UNCOMPRESS ", 512);
	if (attrib&OAPISURFACE_SYSMEM)		 std::strncat(buf, "OAPISURFACE_SYSMEM ", 512);
	LogDbg("BlueViolet", buf);
}

//-------------------------------------------------------------------------------------------
//
void D3D9DebugLog(const char *format, ...)
{
	va_list args;
	va_start(args, format);
    std::vsnprintf(ErrBuf, ERRBUF, format, args);
	va_end(args);

	D3D9DebugQueue.push(std::string(ErrBuf));
}

//-------------------------------------------------------------------------------------------
//
void D3D9DebugLogVec(const char* lbl, oapi::FVECTOR4 &v)
{
    std::snprintf(ErrBuf, ERRBUF, "%s = [%f, %f, %f, %f]", lbl, v.x, v.y, v.z, v.w);
	D3D9DebugQueue.push(std::string(ErrBuf));
}

//-------------------------------------------------------------------------------------------
//
void D3D9InitLog(const char *file)
{
    qpcStart = d3d_clock::now();

    auto d3d9client_log = std::fopen(file, "w+");
	if (!d3d9client_log) { d3d9client_log=nullptr; } // Failed
	else {
        qpcRef = d3d_clock::now();
        std::fprintf(d3d9client_log,"<!DOCTYPE html><html><head><title>D3D9Client Log</title></head><body bgcolor=black text=white>");
        std::fprintf(d3d9client_log,"<center><h2>D3D9Client Log</h2><br>");
        std::fprintf(d3d9client_log,"</center><hr><br><br>");
	}
}

//-------------------------------------------------------------------------------------------
//
void D3D9CloseLog()
{
	if (d3d9client_log) {
        std::fprintf(d3d9client_log,"</body></html>");
        std::fclose(d3d9client_log);
		d3d9client_log = NULL;
	}
}

//-------------------------------------------------------------------------------------------
//
double D3D9GetTime()
{
    auto qpcCurrent = d3d_clock::now();
    return std::chrono::duration_cast<dp_microseconds>
        (qpcCurrent - d3d_clock::time_point{}).count();
}

//-------------------------------------------------------------------------------------------
//
void D3D9SetTime(D3D9Time &inout, double ref)
{
    auto qpcCurrent = d3d_clock::now();
    double time = std::chrono::duration_cast<dp_microseconds>
        (qpcCurrent - d3d_clock::time_point{}).count();
	inout.time += (time - ref);
	inout.count += 1.0;
	inout.peak = max((time - ref), inout.peak);
}

//-------------------------------------------------------------------------------------------
//
char *my_ctime()
{
    auto qpcCurrent = d3d_clock::now();
    double time = std::chrono::duration_cast<dp_milliseconds>
        (qpcCurrent - qpcRef).count();
    double start = std::chrono::duration_cast<dp_seconds>
        (qpcCurrent - qpcStart).count();
    std::snprintf(OprBuf,OPRBUF,"%d: %.1fs %05.2fms", iLine++, start, time);
	qpcRef = qpcCurrent;
	return OprBuf;
}

//-------------------------------------------------------------------------------------------
//
void escape_ErrBuf () {
	std::string buf(ErrBuf);
	size_t n = 0;
	n += replace_all(buf, "&", "&amp;");
	n += replace_all(buf, "<", "&lt;");
	n += replace_all(buf, ">", "&gt;");
	if (n) {
        std::strncpy(ErrBuf,  buf.c_str(), (sizeof(ErrBuf) / sizeof(ErrBuf[0])));
	}
}

//-------------------------------------------------------------------------------------------
//
void LogTrace(const char *format, ...)
{
	if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>3) {
        std::lock_guard g{LogCrit};
		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log, "<font color=Gray>(%s)(0x%lX)</font><font color=DarkGrey> ", my_ctime(), th);

		va_list args;
		va_start(args, format);
        std::vsnprintf(ErrBuf, ERRBUF, format, args);
		va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
	}
}

//-------------------------------------------------------------------------------------------
//
void LogAlw(const char *format, ...)
{
	if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>0) {
        std::lock_guard g {LogCrit};
		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log, "<font color=Gray>(%s)(0x%lX)</font><font color=Olive> ", my_ctime(), th);

		va_list args;
		va_start(args, format);

        std::vsnprintf(ErrBuf, ERRBUF, format, args);

		va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
	}
}

//-------------------------------------------------------------------------------------------
//
void LogDbg(const char *color, const char *format, ...)
{
	if (d3d9client_log == NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>2) {
        std::lock_guard g {LogCrit};

		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log, "<font color=Gray>(%s)(0x%lX)</font><font color=%s> ", my_ctime(), th, color);

		va_list args;
		va_start(args, format);

        std::vsnprintf(ErrBuf, ERRBUF, format, args);

		va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf, d3d9client_log);
		fputs("</font><br>\n", d3d9client_log);
		fflush(d3d9client_log);
	}
}

//-------------------------------------------------------------------------------------------
//
void LogClr(const char *color, const char *format, ...)
{
	if (d3d9client_log == NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>1) {
        std::lock_guard g {LogCrit};

		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log, "<font color=Gray>(%s)(0x%lX)</font><font color=%s> ", my_ctime(), th, color);

		va_list args;
		va_start(args, format);

        std::vsnprintf(ErrBuf, ERRBUF, format, args);

		va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf, d3d9client_log);
		fputs("</font><br>\n", d3d9client_log);
		fflush(d3d9client_log);
	}
}

//-------------------------------------------------------------------------------------------
//
void LogOapi(const char *format, ...)
{

	if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>0) {
        std::lock_guard g {LogCrit};

		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log, "<font color=Gray>(%s)(0x%lX)</font><font color=Olive> ", my_ctime(), th);

		va_list args;
		va_start(args, format);
        std::vsnprintf(ErrBuf, ERRBUF, format, args);
		va_end(args);

		oapiWriteLogV("D3D9: %s", ErrBuf);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
	}
}

// ---------------------------------------------------
//
void LogErr(const char *format, ...)
{
	if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>0) {
        std::lock_guard g {LogCrit};

		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log,"<font color=Gray>(%s)(0x%lX)</font><font color=Red> [ERROR] ", my_ctime(), th);

		va_list args;
		va_start(args, format);
        std::vsnprintf(ErrBuf, ERRBUF, format, args);
		va_end(args);

		oapiWriteLogV("D3D9ERROR: %s", ErrBuf);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
	}
}

// ---------------------------------------------------
//
void LogBlu(const char *format, ...)
{
	if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>1) {
        std::lock_guard g {LogCrit};
		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log,"<font color=Gray>(%s)(0x%lX)</font><font color=#1E90FF> ", my_ctime(), th);

		va_list args;
		va_start(args, format);
        std::vsnprintf(ErrBuf, ERRBUF, format, args);
		va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
	}
}

// ---------------------------------------------------
//
void LogWrn(const char *format, ...)
{
	if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>1) {
        std::lock_guard g {LogCrit};

		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log,"<font color=Gray>(%s)(0x%lX)</font><font color=Yellow> [WARNING] ", my_ctime(), th);

		va_list args;
		va_start(args, format);
        std::vsnprintf(ErrBuf, ERRBUF, format, args);
		va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
		oapiWriteLogV("D3D9Info: %s", ErrBuf);
	}
}

// ---------------------------------------------------
//
void LogBreak(const char* format, ...)
{
	if (d3d9client_log == NULL) return;
	if (iLine > LOG_MAX_LINES) return;
	if (uEnableLog > 1) {
        {
            std::lock_guard g {LogCrit};

            auto th = std::this_thread::get_id();
            fprintf(d3d9client_log, "<font color=Gray>(%s)(0x%lX)</font><font color=Yellow> [WARNING] ", my_ctime(), th);

            va_list args;
            va_start(args, format);
            std::vsnprintf(ErrBuf, ERRBUF, format, args);
            va_end(args);

            escape_ErrBuf();
            fputs(ErrBuf, d3d9client_log);
            fputs("</font><br>\n", d3d9client_log);
            fflush(d3d9client_log);
            oapiWriteLogV("D3D9Debug: %s", ErrBuf);
        }

        /* TODO(jec):  Debug breakpoint trap
		if (Config->DebugBreak) DebugBreak();
        */
	}
}

// ---------------------------------------------------
//
void LogOk(const char *format, ...)
{
	/*if (d3d9client_log==NULL) return;
	if (iLine>LOG_MAX_LINES) return;
	if (uEnableLog>2) {
        std::lock_guard g {LogCrit};

		auto th = std::this_thread::get_id();
		fprintf(d3d9client_log,"<font color=Gray>(%s)(0x%lX)</font><font color=#00FF00> ", my_ctime(), th);

		va_list args;
		va_start(args, format);
        std::vsnprintf(ErrBuf, ERRBUF, format, args);
        va_end(args);

		escape_ErrBuf();
		fputs(ErrBuf,d3d9client_log);
		fputs("</font><br>\n",d3d9client_log);
		fflush(d3d9client_log);
	}*/
}
