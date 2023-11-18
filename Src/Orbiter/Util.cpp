// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#include "Util.h"
/* TODO(jec)
#include <shlobj.h>
*/

#include <filesystem>
#include <chrono>
#include <cctype>
#include <algorithm>
#include <string_view>

LONGLONG NameToId (const char *name)
{
	LONGLONG id = 0;
	const char *c;
	char *cid = (char*)&id;
	int i = 0;
	for (c = name; *c; c++, i++) cid[i%8] += toupper (*c);
	return id;
}

DWORDLONG Str2Crc (const char *str)
{
	DWORDLONG crc = 0;
	for (const char *c = str; *c; c++)
		crc += (DWORDLONG)*c;
	return crc;
}

double rand1()
{
	static double irmax = 1.0/(double)RAND_MAX;
	return (double)rand()*irmax;
}

bool MakePath (const char *fname)
{
    /* TODO(jec):  Is this the correct reimplementation?
	char cbuf[256];
	int i, len = strlen(fname);
	for (i = len; i > 0; i--)
		if (fname[i-1] == '\\') break;
	if (!i) return false;
	if (fname[0] != '\\' && fname[1] != ':') {
		GetCurrentDirectory (256, cbuf);
		len = strlen(cbuf);
		cbuf[len++] = '\\';
	} else len = 0;
	strncpy_s (cbuf+len, 256-len, fname, i);
	int res = SHCreateDirectoryEx (NULL, cbuf, NULL);
	return res == ERROR_SUCCESS;
    */

    return std::filesystem::create_directory({fname});
}

bool iequal(const std::string& s1, const std::string& s2)
{
	unsigned int len = s1.size();
	if (s2.size() != len)
		return false;
	for (unsigned int i = 0; i < len; i++) {
		if (tolower(s1[i]) != tolower(s2[i]))
			return false;
	}
	return true;
}

using tic_clock = std::chrono::high_resolution_clock;
static tic_clock::time_point hi_start {};

void tic()
{
    hi_start = tic_clock::now();
}

double toc()
{
    auto hi_end = tic_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>
        (hi_end - hi_start).count(); // sec
}

/* TODO(jec):  These functions go with widget toolkit */

RECT GetClientPos (HWND hWnd, HWND hChild)
{
	RECT r;
	POINT p;
    /* TODO(jec)
	GetWindowRect (hChild, &r);
	p.x = r.left, p.y = r.top; ScreenToClient (hWnd, &p);
	r.left = p.x, r.top = p.y;
	p.x = r.right, p.y = r.bottom; ScreenToClient (hWnd, &p);
	r.right = p.x, r.bottom = p.y;
    */
	return r;
}

void SetClientPos (HWND hWnd, HWND hChild, RECT &r)
{
    /* TODO(jec)
	MoveWindow (hChild, r.left, r.top, r.right-r.left, r.bottom-r.top, true);
    */
}

bool caseInsensitiveEquals(const std::string_view& str1,
                           const std::string_view& str2) {

    return std::equal(str1.begin(), str1.end(),
                      str2.begin(), str2.end(),
                      [](char c1, char c2) {
                          return std::tolower(static_cast<unsigned char>(c1)) ==
                                 std::tolower(static_cast<unsigned char>(c2));
                      });
}

bool caseInsensitiveStartsWith(const std::string_view& str,
                               const std::string_view& start) {

    return (str.size() >= start.size()) &&
        std::equal(str.begin(), str.begin() + start.size(),
                   start.begin(), start.end(),
                   [](char c1, char c2) {
                       return std::tolower(static_cast<unsigned char>(c1)) ==
                              std::tolower(static_cast<unsigned char>(c2));
                   });
}

