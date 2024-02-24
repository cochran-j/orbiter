/* TextureImage.h
 * Capability to load texture data from file by file-type, and then
 * to convert that texture into a managed IDirect3D9Texture set.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef OVP_D3D9CLIENT_TEXTUREIMAGE_H__
#define OVP_D3D9CLIENT_TEXTUREIMAGE_H__

#include <filesystem>
#include <cstddef>
#include <memory>

#include <d3d9.h>

namespace d3d9client {

class ITextureImage {
public:
    virtual ~ITextureImage() {}

    virtual bool isFileTypeHandled
        (const std::filesystem::path& filepath) const = 0;
    virtual bool isFileTypeHandled
        (const void* fileBuf, std::size_t bufSize) const = 0;

    virtual HRESULT loadFromFile(const std::filesystem::path& filepath) = 0;
    virtual HRESULT loadFromBuffer
        (const void* fileBuf, std::size_t bufSize) = 0;

    virtual HRESULT createTexture
        (struct IDirect3DDevice9* device,
         struct IDirect3DTexture9** texture) const noexcept = 0;

    virtual HRESULT loadIntoTexture
        (struct IDirect3DTexture9* texture) const noexcept = 0;
};


std::unique_ptr<ITextureImage> chooseTextureImage
    (const std::filesystem::path& filepath);


}

#endif /* OVP_D3D9CLIENT_TEXTUREIMAGE_H__ */

