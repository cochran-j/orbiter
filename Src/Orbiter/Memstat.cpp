// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#include "Memstat.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>

class MemStat_impl {
public:
    MemStat_impl();
    ~MemStat_impl();

    long HeapUsage();

private:
    static HMODULE hLib;
    static bool bLib;
    HANDL hProc;
    Proc_GetProcessMemoryInfo pGetProcessMemoryInfo;
    bool active;
};


bool MemStat_impl::bLib = false;
HMODULE MemStat_impl::hLib = 0;

MemStat_impl::MemStat_impl ()
{
	if (!bLib) {
		hLib = LoadLibrary ("Psapi.dll");
		bLib = true;
	}
    hProc = GetCurrentProcess();
    active = (hLib != NULL && hProc != NULL);
	if (active) {
		pGetProcessMemoryInfo = (Proc_GetProcessMemoryInfo)GetProcAddress (hLib, "GetProcessMemoryInfo");
	} else {
		pGetProcessMemoryInfo = 0;
	}
}

MemStat_impl::~MemStat_impl ()
{
    if (hProc) CloseHandle (hProc);
}

long MemStat_impl::HeapUsage ()
{
	if (pGetProcessMemoryInfo) {
	    PROCESS_MEMORY_COUNTERS pmc;
		pGetProcessMemoryInfo (hProc, &pmc, sizeof(pmc));
		return (long)pmc.WorkingSetSize;
	} else return 0;
}

#else /* !_WIN32 */

#include <malloc.h>

class MemStat_impl {
public:
    MemStat_impl();

    long HeapUsage();
};


MemStat_impl::MemStat_impl() {  }

long MemStat_impl::HeapUsage() {
    struct mallinfo2 info = mallinfo2();

    // Narrows on 64-bit builds with 32-bit long.
    return static_cast<long>(info.arena);
}

#endif

MemStat::MemStat()
    : pImpl(std::make_unique<MemStat_impl>()) {

    return;
}

MemStat::MemStat(MemStat&&) = default;
MemStat::~MemStat() = default;
MemStat& MemStat::operator=(MemStat&&) = default;


long MemStat::HeapUsage() {
    return pImpl->HeapUsage();
}

