// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#ifdef _WIN32

#include <windows.h>
#include <process.h>

INT WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR strCmdLine, INT nCmdShow)
{
	const char *cmd = "Modules\\Server\\Orbiter.exe";
	return _execl(cmd, cmd, strCmdLine, NULL);
}


#else /* TODO(jec):  Validate support for non-Linux platforms */

#include <string>
#include <filesystem>
#include <unistd.h>

int main(int /*argc*/, char* argv[]) {

    auto cmdPath = std::filesystem::path{"Modules"} / "Server" / "Orbiter";
#ifdef _WIN32
    cmdPath += ".exe";
#endif

    return execv(cmdPath.c_str(), argv);
}

#endif
