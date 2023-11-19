/* DllCompat.cpp
 *
 * Copyright (c) 2023, Jack Cochran
 * Licensed under the MIT license.
 */

#include "DllCompat.h"

#include <string>
#include <string_view>
#include <filesystem>

namespace DLL {

bool FindStandaloneDll(const std::filesystem::path& path,
                       const std::string_view& moduleName,
                       std::filesystem::path& composedPathOut) {

    composedPathOut = path / DLL::DLLPrefix;
    composedPathOut += moduleName;
    composedPathOut.replace_extension(DLLExt);
    return std::filesystem::exists(composedPathOut);
}

bool FindDllInPluginFolder(const std::filesystem::path& path,
                           const std::string_view& moduleName,
                           std::filesystem::path& composedPathOut) {

    // The idea is that the DLL is in a plugin folder named after the plugin.
    composedPathOut = path / moduleName;
    composedPathOut /= DLL::DLLPrefix;
    composedPathOut += moduleName;
    composedPathOut.replace_extension(DLLExt);
    return std::filesystem::exists(composedPathOut);
}

HINSTANCE LoadDLL(const char* path) {
#ifdef _WIN32
    return LoadLibrary(path);
#else
    return dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
#endif
}

HINSTANCE LoadDLLEx(const char* path) {
#ifdef _WIN32
    return LoadLibraryEx(path, nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                                        LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#else
    // Unsure if we need similar special handling on other platforms
    return dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
#endif
}

// TODO(jec):  LOAD_LIBRARY_AS_DATAFILE  used to extract strings from DLL string
// table resource without mapping the functions into memory or triggering the
// entry point.

void UnloadDLL(HINSTANCE dllToUnload) {
#ifdef _WIN32
    FreeLibrary(dllToUnload);
#else
    dlclose(dllToUnload);
#endif
}

void* GetProcAddress(HINSTANCE dll,
                     const char* procedure) {

#ifdef _WIN32
    return GetProcAddress(dll, procedure);
#else
    return dlsym(dll, procedure);
#endif
}

int GetLastError() {
#ifdef _WIN32
    return GetLastError();
#else
    return errno;
#endif
}

std::string GetDLLFileName(HINSTANCE dll) {
#ifdef _WIN32
    std::string ret {};
    ret.resize(256);
    GetModuleFileName(dll, ret.data(), ret.size());
    return ret;
#else
    std::string ret {};
    link_map* lmap {nullptr};
    dlinfo(dll, RTLD_DI_LINKMAP, &lmap);
    if (lmap) {
        ret = lmap->l_name;
    }
    return ret;
#endif
}



}
