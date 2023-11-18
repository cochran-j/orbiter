// Copyright (c) Martin Schweiger
// Licensed under the MIT License

#include "Help.h"

/* NOTE(jec):  Compat definitions */
#include <windows.h>
struct NMHDR {};
using LPCTSTR = const char*;

#include <htmlhelp.h>
#include <stdio.h>

#include <cctype>
#include <algorithm>
#include <string_view>

bool caseInsensitiveEndsWith(const std::string_view& str,
                             const std::string_view& end) {


    return (str.size() >= end.size()) &&
        (std::equal(str.end() - end.size(), str.end(),
                    end.begin(), end.end(),
                    [](char c1, char c2) {
                        return std::tolower(static_cast<unsigned char>(c1)) ==
                               std::tolower(static_cast<unsigned char>(c2));
                    })
        );
}

void OpenHelp (HWND hWnd, const char *file, const char *topic)
{
	char topic_file[256];
	if (strlen(topic) < 4 || !caseInsensitiveEndsWith(topic, ".htm"))
		sprintf(topic_file, "%s.htm", topic);
	else
		strcpy(topic_file, topic);
    /* TODO(jec):  HtmlHelp doesn't look cross-platform--closed source
	HtmlHelp (hWnd, file, HH_DISPLAY_TOPIC, (DWORD_PTR)topic_file);
    */
}

void OpenDefaultHelp (HWND hWnd, const char *topic)
{
	OpenHelp (hWnd, "html\\orbiter.chm", topic);
}
