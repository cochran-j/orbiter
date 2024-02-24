/* D3DXCompat.cpp
 * Compatibility implementations of D3DX texture and font functions.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

/* TODO(jec):  Compatibility definitions for d3dx9.h */
#define LF_FACESIZE 32
struct TEXTMETRICA;
struct TEXTMETRICW;
#define STDAPI
#define DECLARE_INTERFACE_IID_(type, base, iid) DECLARE_INTERFACE_(type, base)
using LPGUID = void*;
struct IStream;
struct GLYPHMETRICSFLOAT;
using DOUBLE = double;

#include <d3dx9.h>

#include "TextureImage.h"



HRESULT D3DXCreateTexture(struct IDirect3DDevice9* device,
                          UINT width,
                          UINT height,
                          UINT miplevels,
                          DWORD usage,
                          D3DFORMAT format,
                          D3DPOOL pool,
                          struct IDirect3DTexture9** texture) {

    if (!device || !texture) {
        return E_INVALIDARG;
    }

    if ((width == D3DX_DEFAULT) && (height == D3DX_DEFAULT)) {
        width = 256;
        height = 256;
    } else if (width == D3DX_DEFAULT) {
        width = height;
    } else if (height == D3DX_DEFAULT) {
        height = width;
    }

    /* NOTE(jec):  Skipping validation from D3DXCheckTextureRequirements() */

    HRESULT hr = device->CreateTexture
        (width,
         height,
         miplevels,
         usage,
         format,
         pool,
         texture,
         nullptr);

    return hr;
}

HRESULT D3DXCreateTextureFromFileA(struct IDirect3DDevice9* device,
                                   const char* srcFile,
                                   struct IDirect3DTexture9** texture) {

    return D3DXCreateTextureFromFileExA
        (device,
         srcFile,
         D3DX_DEFAULT,
         D3DX_DEFAULT,
         D3DX_DEFAULT,
         0,
         D3DFMT_UNKNOWN,
         D3DPOOL_MANAGED,
         D3DX_DEFAULT,
         D3DX_DEFAULT,
         0,
         nullptr,
         nullptr,
         texture);
}

HRESULT D3DXCreateTextureFromFileExA(struct IDirect3DDevice9* device,
                                     const char* srcFile,
                                     UINT width,
                                     UINT height,
                                     UINT miplevels,
                                     DWORD usage,
                                     D3DFORMAT format,
                                     D3DPOOL pool,
                                     DWORD filter,
                                     DWORD mipFilter,
                                     D3DCOLOR colorKey,
                                     D3DXIMAGE_INFO* srcInfo,
                                     PALETTEENTRY* paletteOut,
                                     struct IDirect3DTexture9** texture) {


    if (!device || !texture) {
        return E_FAIL;
    }

    auto texImage = d3d9client::chooseTextureImage(srcFile);
    if (!texImage) {
        return E_FAIL;
    }

    auto hr = texImage->loadFromFile(srcFile);
    if (hr != S_OK) {
        return hr;
    }

    struct IDirect3DTexture9* mainTexture {nullptr};
    hr = D3DXCreateTexture(device,
                           width,
                           height,
                           miplevels,
                           usage,
                           format,
                           pool,
                           &mainTexture);
    if (hr != S_OK) {
        return hr;
    }

    if (!(pool == D3DPOOL_SYSTEMMEM) &&
        !(pool == D3DPOOL_SCRATCH)) {


        // Load into separate systemmem texture and then UpdateTexture.
        struct IDirect3DTexture9* preloadTexture {nullptr};
        hr = D3DXCreateTexture(device,
                               width,
                               height,
                               miplevels,
                               usage,
                               format,
                               D3DPOOL_SYSTEMMEM,
                               &preloadTexture);
        if (hr != S_OK) {
            mainTexture->Release();
            return hr;
        }

        hr = texImage->loadIntoTexture(preloadTexture);
        if (hr != S_OK) {
            preloadTexture->Release();
            mainTexture->Release();
            return hr;
        }

        hr = device->UpdateTexture(preloadTexture, mainTexture);
        if (hr != S_OK) {
            preloadTexture->Release();
            mainTexture->Release();
            return hr;
        }

        // Good to go now.
        preloadTexture->Release();
        *texture = mainTexture;
        return S_OK;

    } else {

        // Don't need to load into a separate systemmem texture.

        hr = texImage->loadIntoTexture(mainTexture);
        if (hr != S_OK) {
            mainTexture->Release();
            return hr;
        }

        *texture = mainTexture;
        return S_OK;
    }

    // TODO(jec):  Ignoring srcInfo and paletteOut.
}

HRESULT D3DXCreateEffectFromFileA(struct IDirect3DDevice9* /*device*/,
                                  const char* /*srcFile*/,
                                  const D3DXMACRO* /*defines*/,
                                  struct ID3DXInclude* /*include*/,
                                  DWORD /*flags*/,
                                  struct ID3DXEffectPool* /*pool*/,
                                  struct ID3DXEffect** /*effect*/,
                                  struct ID3DXBuffer** /*compilationErrors*/) {

    return E_FAIL;
}

HRESULT D3DXCreateEffectFromFileExA(struct IDirect3DDevice9* /*device*/,
                                    const char* /*srcFile*/,
                                    const D3DXMACRO* /*defines*/,
                                    struct ID3DXInclude* /*include*/,
                                    const char* /*skipConstants*/,
                                    DWORD /*flags*/,
                                    struct ID3DXEffectPool* /*pool*/,
                                    struct ID3DXEffect** /*effect*/,
                                    struct ID3DXBuffer** /*compliationErrors*/) {


    return E_FAIL;
}

HRESULT D3DXGetImageInfoFromFileA(const char* /*srcFile*/,
                                  D3DXIMAGE_INFO* /*srcInfo*/) {

    return E_FAIL;
}

HRESULT D3DXLoadSurfaceFromFileInMemory
    (struct IDirect3DSurface9* /*destSurface*/,
     const PALETTEENTRY* /*destPalette*/,
     const RECT* /*destRect*/,
     const void* /*srcData*/,
     UINT /*srcDataBytes*/,
     const RECT* /*srcRect*/,
     DWORD /*filter*/,
     D3DCOLOR /*colorKey*/,
     D3DXIMAGE_INFO* /*srcInfo*/) {


    return E_FAIL;
}

