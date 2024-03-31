/* DDSTextureImage.h
 * Loader for .dds texture files.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef OVP_D3D9CLIENT_DDSTEXTUREIMAGE_H__
#define OVP_D3D9CLIENT_DDSTEXTUREIMAGE_H__

#include "TextureImage.h"

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <memory>

#include <d3d9.h>

namespace d3d9client {

class DDSTextureImage : public ITextureImage {
public:
    DDSTextureImage();
    ~DDSTextureImage();

    bool isFileTypeHandled
        (const std::filesystem::path& filepath) const override;
    bool isFileTypeHandled
        (const void* fileBuf, std::size_t bufSize) const override;

    HRESULT loadFromFile(const std::filesystem::path& filepath) override;
    HRESULT loadFromBuffer(const void* fileBuf, std::size_t bufSize) override;

    std::uint32_t getWidth() const override;
    std::uint32_t getHeight() const override;
    std::uint32_t getMipLevels() const override;
    D3DFORMAT getFormat() const override;

    HRESULT createTexture
        (struct IDirect3DDevice9* device,
         struct IDirect3DTexture9** texture) const noexcept override;
    HRESULT loadIntoTexture
        (struct IDirect3DTexture9* texture) const noexcept override;


private:
    struct impl;
    std::unique_ptr<impl> pImpl;
};


}

#endif /* OVP_D3D9CLIENT_DDSTEXTUREIMAGE_H__ */

