#ifndef DDSREAD_H
#define DDSREAD_H

#include "imagetools.h"
#include <cstdint>

using BYTE = std::uint8_t;

Image ddsread(const char *fname);
Image ddsscan(const BYTE *data, int ndata);

#endif // !DDSREAD_H
