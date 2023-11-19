// DllCompat.h
// Provides definitions for LoadLibrary and GetProcAddress, even
// on non-Windows systems.
//
// Copyright (c) 2023, Jack Cochran
// Licensed under the MIT license.

#ifndef ORBITERSDK_DLLCOMPAT_H__
#define ORBITERSDK_DLLCOMPAT_H__

#ifdef _WIN32
#include <windows.h>

#define DLLENTER
#define DLLEXIT

namespace DLL {
    const char *const DLLPrefix = "";
    const char *const DLLExt = "dll";
}

inline HINSTANCE DllGetInstance(void* fcn) {
    return NULL;
}

// TODO:  Pickup some symbols to narrow scope to POSIX-ish
#else

#include <dlfcn.h>

#define DLLENTER __attribute__ ((constructor))
#define DLLEXIT __attribute__ ((destructor))

namespace DLL {
    const char *const DLLPrefix = "lib";
    const char *const DLLExt = "so"; // TODO(jec): other extensions, MacOS?
}

// WARNING:  This function relies on dlopen returning the same handle on nested
// calls.  It appears that this behavior is specified in the man pages.
inline void* DllGetInstance(void* fcn) {
    Dl_info info;
    int ret = dladdr(fcn, &info);
    if (ret == 0) {
        return nullptr;
    }

    void* handle = dlopen(info.dli_fname, RTLD_LAZY | RTLD_NOLOAD);
    dlclose(handle);

    return handle;
}

inline void* GetModuleHandle(const char* libName) {
    void* handle = dlopen(libName, RTLD_LAZY | RTLD_NOLOAD);
    dlclose(handle);
    return handle;
}

inline void* LoadLibrary(const char* libName) {
    return dlopen(libName, RTLD_LAZY | RTLD_GLOBAL);
}

inline void* GetProcAddress(void* hModule, LPCSTR lpProcName) {
    return dlsym(hModule, lpProcName);
}

inline void FreeLibrary(void* hModule) {
    dlclose(hModule);
}


#endif

#endif

