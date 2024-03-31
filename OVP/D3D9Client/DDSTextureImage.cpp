/* DDSTextureImage.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#include "DDSTextureImage.h"

#include <filesystem>
#include <fstream>
#include <exception>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <cstdint>

#include <d3d9.h>

namespace d3d9client {


/* CREDIT:  DirectXTK toolkit:  https://github.com/microsoft/DirectXTK */
/* Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License. */
constexpr std::uint32_t DDS_MAGIC = 0x20534444; // "DDS "

constexpr std::uint32_t DDS_FOURCC = 0x00000004;
constexpr std::uint32_t DDS_RGB = 0x00000040;
/* constexpr std::uint32_t DDS_RGBA = 0x00000041; */
constexpr std::uint32_t DDS_LUMINANCE = 0x00020000;
/* constexpr std::uint32_t DDS_LUMINANCEA = 0x00020001; */
/* constexpr std::uint32_t DDS_ALPHAPIXELS = 0x00000001; */
constexpr std::uint32_t DDS_ALPHA = 0x00000002;
/* constexpr std::uint32_t DDS_PAL8 = 0x00000020; */
/* constexpr std::uint32_t DDS_PAL8A = 0x00000021; */
constexpr std::uint32_t DDS_BUMPDUDV = 0x00080000;

/*
constexpr std::uint32_t DDS_HEADER_FLAGS_TEXTURE = 0x00001007;
constexpr std::uint32_t DDS_HEADER_FLAGS_MIPMAP = 0x00020000;
*/
constexpr std::uint32_t DDS_HEADER_FLAGS_VOLUME = 0x00800000;
/*
constexpr std::uint32_t DDS_HEADER_FLAGS_PITCH = 0x00000008;
constexpr std::uint32_t DDS_HEADER_FLAGS_LINEARSIZE = 0x00080000;
*/

struct DDS_PIXELFORMAT {
    std::uint32_t size;
    std::uint32_t flags;
    std::uint32_t fourCC;
    std::uint32_t RGBBitCount;
    std::uint32_t RBitMask;
    std::uint32_t GBitMask;
    std::uint32_t BBitMask;
    std::uint32_t ABitMask;
};

struct DDS_HEADER {
    std::uint32_t magic;  // Just put the magic number in the header.
    std::uint32_t size;
    std::uint32_t flags;
    std::uint32_t height;
    std::uint32_t width;
    std::uint32_t pitchOrLinearSize;
    std::uint32_t depth;

    std::uint32_t mipMapCount;
    std::uint32_t reserved1[11];
    DDS_PIXELFORMAT ddspf;
    std::uint32_t caps;
    std::uint32_t caps2;
    std::uint32_t caps3;
    std::uint32_t caps4;
    std::uint32_t reserved2;
};

/* Somewhat based on GetDXGIFormat() from DirectXTK */
#define ISBITMASK(r, g, b, a) (\
    ddpf.RBitMask == r &&\
    ddpf.GBitMask == g &&\
    ddpf.BBitMask == b &&\
    ddpf.ABitMask == a\
)

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)\
    (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch0))\
    | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch1)) << 8)\
    | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch2)) << 16)\
    | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch3)) << 24))
#endif /* MAKEFOURCC */

D3DFORMAT getD3DFormat(const DDS_PIXELFORMAT& ddpf) noexcept
{
    if (ddpf.flags & DDS_RGB)
    {
        // Note that sRBD formats are written using the "DX10" extended header.
        // Note(jec):  We aren't implementing support for the DX10 extended
        // header.

        switch (ddpf.RGBBitCount)
        {
        case 32:
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return D3DFMT_A8B8G8R8;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
            {
                return D3DFMT_A8R8G8B8;
            }

            if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0))
            {
                return D3DFMT_X8R8G8B8;
            }

            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0))
            {
                return D3DFMT_X8B8G8R8;
            }

            if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return D3DFMT_A2B10G10R10;
            }

            if (ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000))
            {
                return D3DFMT_A2R10G10B10;
            }

            if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
            {
                return D3DFMT_V16U16;
            }

            if (ISBITMASK(0xffffffff, 0, 0, 0))
            {
                return D3DFMT_R32F;
            }

            break;

        case 24:
            if (ISBITMASK(0x0000ff, 0x00ff00, 0xff0000, 0))
            {
                return D3DFMT_R8G8B8;
            }

        case 16:
            if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
            {
                return D3DFMT_A1R5G5B5;
            }

            if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0))
            {
                return D3DFMT_R5G6B5;
            }

            if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0))
            {
                return D3DFMT_X1R5G5B5;
            }

            if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
            {
                return D3DFMT_A4R4G4B4;
            }

            // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
            if (ISBITMASK(0x00ff, 0, 0, 0xff00))
            {
                return D3DFMT_A8L8;
            }

            if (ISBITMASK(0xffff, 0, 0, 0))
            {
                return D3DFMT_L16;
            }

            if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0))
            {
                return D3DFMT_A8P8;
            }

            break;

        case 8:
            // NVTT versions 1.x wrote this as RGB instead of LUMINANCE
            if (ISBITMASK(0xff, 0, 0, 0)) {
                return D3DFMT_L8;
            }

            // Skipping 3:3:2 and palleted formats:  D3DFMT_R3G3B2, D3DFMT_P8, etc.

            break;

        default:
            return D3DFMT_UNKNOWN;
        }
    }
    else if (ddpf.flags & DDS_LUMINANCE)
    {
        switch (ddpf.RGBBitCount)
        {
        case 16:
            if (ISBITMASK(0xffff, 0, 0, 0))
            {
                return D3DFMT_L16;
            }

            if (ISBITMASK(0x00ff, 0, 0, 0xff00))
            {
                return D3DFMT_A8L8;
            }
            break;

        case 8:
            if (ISBITMASK(0xff, 0, 0, 0))
            {
                return D3DFMT_L8;
            }

            if (ISBITMASK(0x0f, 0, 0, 0xf0))
            {
                return D3DFMT_A4L4;
            }

            if (ISBITMASK(0x00ff, 0, 0, 0xff00))
            {
                return D3DFMT_A8L8; // Some DDS writers assume that the bitcount should be 8 instead of 16
            }
            break;

        default:
            return D3DFMT_UNKNOWN;
        }
    }
    else if (ddpf.flags & DDS_ALPHA)
    {
        if (8 == ddpf.RGBBitCount)
        {
            return D3DFMT_A8;
        }
    }
    else if (ddpf.flags & DDS_BUMPDUDV)
    {
        switch (ddpf.RGBBitCount)
        {
        case 32:
            if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return D3DFMT_Q8W8V8U8;
            }

            if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))
            {
                return D3DFMT_V16U16;
            }

            if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return D3DFMT_A2W10V10U10;
            }
            break;

        case 16:
            if (ISBITMASK(0x00ff, 0xff00, 0, 0))
            {
                return D3DFMT_V8U8;
            }
            break;

        default:
            return D3DFMT_UNKNOWN;
        }

        // Skipping D3DFMT_L6V5U5 and D3DFMT_X8L8V8U8
    }
    else if (ddpf.flags & DDS_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
        {
            return D3DFMT_DXT1;
        }
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
        {
            return D3DFMT_DXT3;
        }
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
        {
            return D3DFMT_DXT5;
        }

        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
        {
            return D3DFMT_DXT2;
        }
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
        {
            return D3DFMT_DXT4;
        }

        /* Not exist
        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
        {
            return D3DFMT_ATI1;
        }
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
        {
            return D3DFMT_BC4U;
        }
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
        {
            return D3DFMT_BC4S;
        }

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
        {
            return D3DFMT_ATI2;
        }
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
        {
            return D3DFMT_BC5U;
        }
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
        {
            return D3DFMT_BC5S;
        }

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
        {
            return D3DFMT_RGBG;
        }
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
        {
            return D3DFMT_GRGB;
        }
        */

        if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
        {
            return D3DFMT_YUY2;
        }

        // Random other 4ccs.
        switch (ddpf.fourCC)
        {
        case 36:
            return D3DFMT_A16B16G16R16;

        case 110:
            return D3DFMT_Q16W16V16U16;

        case 111:
            return D3DFMT_R16F;

        case 112:
            return D3DFMT_G16R16F;

        case 113:
            return D3DFMT_A16B16G16R16F;

        case 114:
            return D3DFMT_R32F;

        case 115:
            return D3DFMT_G32R32F;

        case 116:
            return D3DFMT_A32B32G32R32F;

        default:
            return D3DFMT_UNKNOWN;
        }
    }

    return D3DFMT_UNKNOWN;
}
#undef ISBITMASK


/* END CREDIT:  DirectXTK toolkit */

struct DDSTextureImage::impl {
    bool isEmpty;
    DDS_HEADER header;
    std::vector<unsigned char> bit_data;

    static bool isDDSHeaderValid(const DDS_HEADER& header);
    static DDS_HEADER loadHeaderFromFile(const std::filesystem::path& filepath);
    void loadInHeaderFromFile(const std::filesystem::path& filepath);
    void loadBitDataFromFile(const std::filesystem::path& filepath);

    void loadHeaderFromBuffer(const void* fileBuf, std::size_t bufSize);
    void loadBitDataFromBuffer(const void* fileBuf, std::size_t bufSize);
};

bool DDSTextureImage::impl::isDDSHeaderValid(const DDS_HEADER& header) {
    if (header.magic != DDS_MAGIC) {
        return false;
    }

    /* NOTE(jec):  Header size field doesn't count magic as part of header.*/
    if (header.size != sizeof(DDS_HEADER) - sizeof(std::uint32_t)) {
        return false;
    }

    if (header.ddspf.size != sizeof(DDS_PIXELFORMAT)) {
        return false;
    }

    return true;
}

DDS_HEADER DDSTextureImage::impl::loadHeaderFromFile
    (const std::filesystem::path& filepath) {

    DDS_HEADER header {};
    std::ifstream is {filepath, std::ios::in | std::ios::binary};
    is.read(reinterpret_cast<char*>(&header),
            sizeof(DDS_HEADER));

    return header;
}

void DDSTextureImage::impl::loadInHeaderFromFile
    (const std::filesystem::path& filepath) {

    header = loadHeaderFromFile(filepath);
}

void DDSTextureImage::impl::loadBitDataFromFile
    (const std::filesystem::path& filepath) {

    auto file_size = std::filesystem::file_size(filepath);
    auto bit_data_size = file_size - sizeof(DDS_HEADER);

    bit_data.resize(bit_data_size);

    std::ifstream is {filepath, std::ios::in | std::ios::binary};
    is.seekg(sizeof(DDS_HEADER));
    is.read(reinterpret_cast<char*>(bit_data.data()), bit_data_size);
}

void DDSTextureImage::impl::loadHeaderFromBuffer
    (const void* fileBuf, std::size_t bufSize) {

    std::copy(reinterpret_cast<const unsigned char*>(fileBuf),
              reinterpret_cast<const unsigned char*>(fileBuf) +
                sizeof(DDS_HEADER),
              reinterpret_cast<unsigned char*>(&header));
}

void DDSTextureImage::impl::loadBitDataFromBuffer
    (const void* fileBuf, std::size_t bufSize) {

    std::copy(reinterpret_cast<const unsigned char*>(fileBuf) +
                sizeof(DDS_HEADER),
              reinterpret_cast<const unsigned char*>(fileBuf) + bufSize,
              bit_data.begin());
}

DDSTextureImage::DDSTextureImage()
    : pImpl{std::make_unique<impl>()} {

    pImpl->isEmpty = true;
}

DDSTextureImage::~DDSTextureImage() = default;

bool DDSTextureImage::isFileTypeHandled
    (const std::filesystem::path& filepath) const {

    try {

        if ((filepath.extension() != ".dds") &&
            (filepath.extension() != ".DDS")) {

            return false;
        }

        if (!std::filesystem::exists(filepath)) {
            return false;
        }

        if (std::filesystem::file_size(filepath) < sizeof(DDS_HEADER)) {
            return false;
        }

        DDS_HEADER ddsHeader {};
        {
            std::ifstream is {filepath, std::ios::in | std::ios::binary};
            is.read(reinterpret_cast<char*>(&ddsHeader),
                    sizeof(ddsHeader));
        }

        return impl::isDDSHeaderValid(ddsHeader);

    } catch (const std::exception&) {
        return false;
    }
}

bool DDSTextureImage::isFileTypeHandled
    (const void* fileBuf, std::size_t bufSize) const {

    if (bufSize < sizeof(DDS_HEADER)) {
        return false;
    }

    DDS_HEADER hdr {};
    std::copy(reinterpret_cast<const unsigned char*>(fileBuf),
              reinterpret_cast<const unsigned char*>(fileBuf) + sizeof(DDS_HEADER),
              reinterpret_cast<unsigned char*>(&hdr));

    return impl::isDDSHeaderValid(hdr);
}

HRESULT DDSTextureImage::loadFromFile
    (const std::filesystem::path& filepath) {

    try {
        pImpl->loadInHeaderFromFile(filepath);
        if (!impl::isDDSHeaderValid(pImpl->header)) {
            return E_FAIL;
        }

        pImpl->loadBitDataFromFile(filepath);
        pImpl->isEmpty = false;
    } catch (const std::exception&) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT DDSTextureImage::loadFromBuffer
    (const void* fileBuf, std::size_t bufSize) {

    if (bufSize < sizeof(DDS_HEADER)) {
        return E_INVALIDARG;
    }

    try {
        pImpl->loadHeaderFromBuffer(fileBuf, bufSize);
        if (!impl::isDDSHeaderValid(pImpl->header)) {
            return E_FAIL;
        }

        pImpl->loadBitDataFromBuffer(fileBuf, bufSize);
        pImpl->isEmpty = false;
    } catch (const std::exception&) {
        return E_FAIL;
    }

    return E_FAIL;
}

std::uint32_t DDSTextureImage::getWidth() const {
    return pImpl->header.width;
}

std::uint32_t DDSTextureImage::getHeight() const {
    return pImpl->header.height;
}

std::uint32_t DDSTextureImage::getDepth() const {
    return pImpl->header.depth;
}

std::uint32_t DDSTextureImage::getMipLevels() const {
    return pImpl->header.mipMapCount;
}

D3DFORMAT DDSTextureImage::getFormat() const {
    return getD3DFormat(pImpl->header.ddspf);
}

HRESULT DDSTextureImage::createTexture
    (struct IDirect3DDevice9* device,
     struct IDirect3DTexture9** texture) const noexcept {


    return E_FAIL;
}

HRESULT DDSTextureImage::loadIntoTexture
    (struct IDirect3DTexture9* texture) const noexcept {

    if (pImpl->isEmpty) {
        return E_FAIL;
    }

    if (!impl::isDDSHeaderValid(pImpl->header)) {
        return E_FAIL;
    }

    if (!texture) {
        return E_FAIL;
    }

    auto width = pImpl->header.width;
    auto height = pImpl->header.height;
    /* auto depth = pImpl->header.depth; */ // TODO(jec):  What use?
    auto mipcount = pImpl->header.mipMapCount;
    if (mipcount == 0) {
        mipcount = 1;
    }

    auto d3dFormat = getD3DFormat(pImpl->header.ddspf);
    if (d3dFormat == D3DFMT_UNKNOWN) {
        return E_FAIL;
    }

    if (pImpl->header.flags & DDS_HEADER_FLAGS_VOLUME) {
        // Need to load into an IDirect3DVolumeTexture9.
        return E_FAIL;
    }

    // Check matching texture parameters:
    if (texture->GetLevelCount() != mipcount) {
        return E_FAIL;
    }

    D3DSURFACE_DESC surfDesc {};
    auto hr = texture->GetLevelDesc(0, &surfDesc);
    if (hr != S_OK) {
        return hr;
    }

    if (d3dFormat != surfDesc.Format) {
        return E_FAIL;
    }
    if (width != surfDesc.Width) {
        return E_FAIL;
    }
    if (height != surfDesc.Height) {
        return E_FAIL;
    }

    // TODO(jec):  Skipping cubemap support:  IDirect3DCubeTexture9.

    // TODO(jec):  Skipping security checks on number of miplevels.

    // TODO(jec):  Skipping more dimension and size checks.

    // TODO the best way to actually load into a pre-existing texture is to 
    // create a new texture, lock rectangle, copy bits, and UpdateTexture.
    // MANAGED and DEFAULT pools cannot be locked and accessed directly.
    // We need an IDirect3DDevice9.
    //
    // UPDATE:  This function requires a texture loaded into a lockable area.

    D3DLOCKED_RECT rect {};
    hr = texture->LockRect(0, &rect, nullptr, 0);
    if (hr != S_OK) {
        return hr;
    }

    std::copy(pImpl->bit_data.cbegin(),
              pImpl->bit_data.cend(),
              reinterpret_cast<unsigned char*>(rect.pBits));

    hr = texture->UnlockRect(0);
    if (hr != S_OK) {
        return hr;
    }

    // TODO:  Assume we need to autogen mipmaps?
    if (mipcount == 1) {
        texture->GenerateMipSubLevels();
    }

    return S_OK;
}


}

