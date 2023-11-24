/* DllCompat.h
 * Platform abstractions for DLL loading, unloading.
 *
 * Copyright (c) 2023, Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef __DLLCOMPAT_H
#define __DLLCOMPAT_H

#include <string>
#include <string_view>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <link.h>
#include <errno.h>
#endif

namespace DLL {

#ifdef _WIN32
constexpr std::string_view DLLPrefix = "";
constexpr std::string_view DLLExt = ".dll";
#else
using HINSTANCE = void*;
constexpr std::string_view DLLPrefix = "lib";
constexpr std::string_view DLLExt = ".so";
#endif

bool FindStandaloneDll(const std::filesystem::path& path,
                       const std::string_view& moduleName,
                       std::filesystem::path& composedPathOut);

bool FindDllInPluginFolder(const std::filesystem::path& path,
                           const std::string_view& moduleName,
                           std::filesystem::path& composedPathOut);


HINSTANCE LoadDLL(const char* path);

// Does some path lookup flags needed on Windows
// See https://stackoverflow.com/questions/36275535/loadlibraryex-error-87-the-parameter-is-incorrect
HINSTANCE LoadDLLEx(const char* path);

void UnloadDLL(HINSTANCE dllToUnload);

void* GetProcAddress(HINSTANCE dll,
                     const char* procedure);

std::string GetDLLFileName(HINSTANCE dll);

int GetLastError();

}


#endif /* __DLLCOMPAT_H */

