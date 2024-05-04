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

#include <cstddef>
#include <cmath>
#include <vector>
#include <fstream>
#include <d3dx9.h>

#include "TextureImage.h"
#include "DxcCompat.h"
/* WARNING:  DirectXMath is not a nice header and has to go last. */
#include "DirectXMath.h"
#include "DirectXCollision.h"
#include "dxc/dxcapi.h"


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

    if (miplevels == D3DX_DEFAULT) {
        miplevels = 0;
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

HRESULT D3DXCreateVolumeTexture(struct IDirect3DDevice9* device,
                                UINT width,
                                UINT height,
                                UINT depth,
                                UINT miplevels,
                                DWORD usage,
                                D3DFORMAT format,
                                D3DPOOL pool,
                                struct IDirect3DVolumeTexture9** texture) {

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

    if (miplevels == D3DX_DEFAULT) {
        miplevels = 0;
    }

    /* NOTE(jec):  Skipping validation checks */

    HRESULT hr = device->CreateVolumeTexture
        (width,
         height,
         depth,
         miplevels,
         usage,
         format,
         pool,
         texture,
         nullptr);

    return hr;
}

HRESULT D3DXCreateCubeTexture(struct IDirect3DDevice9* device,
                              UINT size,
                              UINT miplevels,
                              DWORD usage,
                              D3DFORMAT format,
                              D3DPOOL pool,
                              struct IDirect3DCubeTexture9** texture) {


    if (!device || !texture) {
        return E_INVALIDARG;
    }

    if (miplevels == D3DX_DEFAULT) {
        miplevels = 0;
    }

    /* NOTE(jec):  Skipping validation checks */

    HRESULT hr = device->CreateCubeTexture
        (size,
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
         /* This is D3DPOOL_MANAGED in the MSDN docs.  But MANAGED pool is not
          * valid with IDirect3dDevice9 */
         D3DPOOL_DEFAULT,
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

    if ((width == 0) || (width == D3DX_DEFAULT)) {
        width = texImage->getWidth();
    }

    if ((height == 0) || (height == D3DX_DEFAULT)) {
        height = texImage->getHeight();
    }

    if ((miplevels == 0) || (miplevels == D3DX_DEFAULT)) {
        miplevels = texImage->getMipLevels();
    }

    if (format == D3DFMT_UNKNOWN) {
        format = texImage->getFormat();
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

HRESULT D3DXCreateTextureFromFileInMemoryEx(struct IDirect3DDevice9* /*device*/,
                                            const void* /*srcData*/,
                                            UINT /*srcDataSize*/,
                                            UINT /*width*/,
                                            UINT /*height*/,
                                            UINT /*miplevels*/,
                                            DWORD /*usage*/,
                                            D3DFORMAT /*format*/,
                                            D3DPOOL /*pool*/,
                                            DWORD /*filter*/,
                                            DWORD /*mipfilter*/,
                                            D3DCOLOR /*colorkey*/,
                                            D3DXIMAGE_INFO* /*srcInfo*/,
                                            PALETTEENTRY* /*palette*/,
                                            struct IDirect3DTexture9** texture) {
    return E_FAIL;
}

HRESULT D3DXSaveTextureToFileA(const char* /*destFile*/,
                               D3DXIMAGE_FILEFORMAT /*imageFormat*/,
                               struct IDirect3DBaseTexture9* /*srcTexture*/,
                               const PALETTEENTRY* /*srcPalette*/) {

    return E_FAIL;
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

HRESULT D3DXGetImageInfoFromFileA(const char* srcFile,
                                  D3DXIMAGE_INFO* srcInfo) {

    if (!srcFile || !srcInfo) {
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

    srcInfo->Width = texImage->getWidth();
    srcInfo->Height = texImage->getHeight();
    srcInfo->Depth = texImage->getDepth();
    srcInfo->MipLevels = texImage->getMipLevels();
    srcInfo->Format = texImage->getFormat();
    srcInfo->ResourceType = D3DRTYPE_TEXTURE; // TODO: cube and volume textures
    srcInfo->ImageFileFormat = D3DXIFF_DDS; // TODO: Other file formats.

    return S_OK;
}

HRESULT D3DXLoadSurfaceFromFileA(struct IDirect3DSurface9* /*destSurface*/,
                                 const PALETTEENTRY* /*destPalette*/,
                                 const RECT* /*destRect*/,
                                 const char* /*srcFile*/,
                                 const RECT* /*srcRect*/,
                                 DWORD /*filter*/,
                                 D3DCOLOR /*colorKey*/,
                                 D3DXIMAGE_INFO* /*srcInfo*/) {


    return E_FAIL;
}

HRESULT D3DXSaveSurfaceToFileA(const char* /*destFile*/,
                               D3DXIMAGE_FILEFORMAT /*destFormat*/,
                               struct IDirect3DSurface9* /*srcSurface*/,
                               const PALETTEENTRY* /*srcPalette*/,
                               const RECT* /*srcRect*/) {

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

HRESULT D3DXLoadSurfaceFromSurface(struct IDirect3DSurface9* /*destSurface*/,
                                   const PALETTEENTRY* /*destPaletteEntry */,
                                   const RECT* /*destRect*/,
                                   struct IDirect3DSurface9* /*srcSurface*/,
                                   const PALETTEENTRY* /*srcPaletteEntry */,
                                   const RECT* /*srcRect*/,
                                   DWORD /*filter*/,
                                   D3DCOLOR /*colorKey*/) {


    return E_FAIL;
}



HRESULT D3DXComputeBoundingSphere(const D3DXVECTOR3* firstPosition,
                                  DWORD numVertices,
                                  DWORD dwStride,
                                  D3DXVECTOR3* center,
                                  float* radius) {

    DirectX::BoundingSphere bs {};
    bs.CreateFromPoints(bs,
                        numVertices,
                        reinterpret_cast<const DirectX::XMFLOAT3*>(firstPosition),
                        dwStride);

    if (center) {
        center->x = bs.Center.x;
        center->y = bs.Center.y;
        center->z = bs.Center.z;
    }

    if (radius) {
        *radius = bs.Radius;
    }
    return S_OK;
}

HRESULT D3DXCompileShaderFromFileA(const char* srcFile,
                                   const D3DXMACRO* /*defines*/,
                                   struct ID3DXInclude* /*include*/,
                                   const char* /*functionName*/,
                                   const char* /*profile*/,
                                   DWORD /*flags*/,
                                   struct ID3DXBuffer** shader,
                                   struct ID3DXBuffer** errorMsgs,
                                   struct ID3DXConstantTable** constantTable) {
    if (!srcFile) {
        return E_FAIL;
    }

    std::ifstream ifs {srcFile};
    if (!ifs.is_open()) {
        return E_FAIL;
    }

    /* Read srcFile into DxcBuffer. */
    d3d9client::StringDxcBuffer srcFileBuf {ifs};
    ifs.close();

    /* Create an IDxcCompiler3 */
    IDxcCompiler3* compiler {nullptr};
    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler,
                                   uuidof(IDxcCompiler3),
                                   reinterpret_cast<void**>(&compiler));

    if (hr != S_OK) {
        return hr;
    }
    if (!compiler) {
        return E_FAIL;
    }

    /* Function name, profile and flags? */
    /* Deal with includes -- Not actually in use in this program. */

    /* Get an IDxcResult */
    IDxcResult* result {nullptr};
    hr = compiler->Compile(&srcFileBuf.dxcBuffer(),
                           nullptr, // no arguments
                           0,       // arg count
                           nullptr, // No include handler.
                           uuidof(IDxcResult),
                           reinterpret_cast<void**>(&result));

    if (hr != S_OK) {
        compiler->Release();
        return hr;
    }
    if (!result) {
        compiler->Release();
        return E_FAIL;
    }

    /* Map back to ID3DXBuffers */
    if (shader) {
        IDxcBlob* shaderBlob {nullptr};
        hr = result->GetOutput(DXC_OUT_OBJECT,
                               uuidof(IDxcBlob),
                               reinterpret_cast<void**>(&shaderBlob),
                               nullptr);
        if (hr != S_OK) {
            result->Release();
            compiler->Release();
            return hr;
        }
        if (!shaderBlob) {
            result->Release();
            compiler->Release();
            return E_FAIL;
        }

        auto shaderBuffer = new d3d9client::DxcBlob_D3DXBuffer{shaderBlob};
        shaderBlob->Release(); // We no longer need shaderBlob.
        *shader = shaderBuffer; // Passing ownership.
    }

    if (errorMsgs) {
        IDxcBlobUtf8* errorMsgBlob {nullptr};
        hr = result->GetOutput(DXC_OUT_ERRORS,
                               uuidof(IDxcBlobUtf8),
                               reinterpret_cast<void**>(&errorMsgBlob),
                               nullptr);
        if (hr != S_OK) {
            result->Release();
            compiler->Release();
            return hr;
        }
        if (!errorMsgBlob) {
            result->Release();
            compiler->Release();
            return hr;
        }

        auto errorMsgBuffer =
            new d3d9client::DxcBlobUtf8_D3DXBuffer{errorMsgBlob};
        errorMsgBlob->Release(); // We no longer need errorMsgBlob.
        *errorMsgs = errorMsgBuffer; // Passing ownership.
    }

    if (constantTable) {
        /* TODO(jec):  Do we actually need this?  It's a lot of methods. */
    }

    return E_FAIL;
}

HRESULT D3DXDisassembleShader(const DWORD* /*shader*/,
                              int /*enableColorCode*/,
                              const char* /*comments*/,
                              struct ID3DXBuffer** /*disassembly*/) {


    return E_FAIL;
}

HRESULT D3DXDisassembleEffect(struct ID3DXEffect* /*effect*/,
                              int /*enableColorCode*/,
                              struct ID3DXBuffer** /*disassembly*/) {

    return E_FAIL;
}


/* D3DXVECTOR4 */
/*
 * struct D3DXVECTOR4 {
 *     float x;
 *     float y;
 *     float z;
 *     float w;
 * };
 */

/* inline
D3DXVECTOR4::D3DXVECTOR4(const float* fpArray)
    : x{fpArray[0]},
      y{fpArray[1]},
      z{fpArray[2]},
      w{fpArray[3]}   {  }

D3DXVECTOR4::D3DXVECTOR4(const D3DXFLOAT16* fpArray)
    : x{fpArray[0]},
      y{fpArray[1]},
      z{fpArray[2]},
      w{fpArray[3]}   {  }

D3DXVECTOR4::D3DXVECTOR4(const D3DVECTOR& xyz, float w_)
    : x{xyz.x},
      y{xyz.y},
      z{xyz.z},
      w{w_} {  }

D3DXVECTOR4::D3DXVECTOR4(float x_, float y_, float z_, float w_)
    : x{x_},
      y{y_},
      z{z_},
      w{w_} {  }
*/

DirectX::XMMATRIX toXmMatrix(const D3DXMATRIX& in) {
    DirectX::XMMATRIX ret {
        in(0, 0),
        in(0, 1),
        in(0, 2),
        in(0, 3),

        in(1, 0),
        in(1, 1),
        in(1, 2),
        in(1, 3),

        in(2, 0),
        in(2, 1),
        in(2, 2),
        in(2, 3),

        in(3, 0),
        in(3, 1),
        in(3, 2),
        in(3, 3)
    };

    return ret;
}

DirectX::XMVECTOR toXmVector(const D3DXVECTOR3& in) {
    return {in.x, in.y, in.z, 0.0f};
}

void toD3DXMatrix(D3DXMATRIX& out, const DirectX::XMMATRIX& in) {
    DirectX::XMFLOAT4X4 floatIn {};
    DirectX::XMStoreFloat4x4(&floatIn, in);

    out(0, 0) = floatIn(0, 0);
    out(0, 1) = floatIn(0, 1);
    out(0, 2) = floatIn(0, 2);
    out(0, 3) = floatIn(0, 3);

    out(1, 0) = floatIn(1, 0);
    out(1, 1) = floatIn(1, 1);
    out(1, 2) = floatIn(1, 2);
    out(1, 3) = floatIn(1, 3);

    out(2, 0) = floatIn(2, 0);
    out(2, 1) = floatIn(2, 1);
    out(2, 2) = floatIn(2, 2);
    out(2, 3) = floatIn(2, 3);

    out(3, 0) = floatIn(3, 0);
    out(3, 1) = floatIn(3, 1);
    out(3, 2) = floatIn(3, 2);
    out(3, 3) = floatIn(3, 3);
}


D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* out,
                              float* determinant,
                              const D3DXMATRIX* in) {

    if (!in || !out) {
        return out;
    }

    auto xmIn = toXmMatrix(*in);

    DirectX::XMVECTOR xmDeterminant {};
    auto xmOut = DirectX::XMMatrixInverse(&xmDeterminant, xmIn);

    if (determinant) {
        for (std::size_t i = 0; i < 3; ++i) {
            determinant[i] = DirectX::XMVectorGetByIndex(xmDeterminant, i);
        }
    }
    toD3DXMatrix(*out, xmOut);
    return out;
}

D3DXMATRIX* D3DXMatrixRotationAxis(D3DXMATRIX* out,
                                   const D3DXVECTOR3* v,
                                   float angle_rad) {

    if (!out || !v) {
        return out;
    }

    D3DXVECTOR3 v_norm {};
    D3DXVec3Normalize(&v_norm, v);

    float c = std::cos(angle_rad);
    float s = std::sin(angle_rad);

    // https://en.wikipedia.org/wiki/Rotation_matrix

    (*out)(0, 0) = c + v_norm.x * v_norm.x * (1.0f - c);
    (*out)(0, 1) = v_norm.x * v_norm.y * (1.0f - c) - v_norm.z * s;
    (*out)(0, 2) = v_norm.x * v_norm.z * (1.0f - c) + v_norm.y * s;
    (*out)(0, 3) = 0.0f;

    (*out)(1, 0) = v_norm.y * v_norm.x * (1.0f - c) + v_norm.z * s;
    (*out)(1, 1) = c + v_norm.y * v_norm.y * (1.0f - c);
    (*out)(1, 2) = v_norm.y * v_norm.z * (1.0f - c) - v_norm.x * s;
    (*out)(1, 3) = 0.0f;

    (*out)(2, 0) = v_norm.z * v_norm.x * (1.0f - c) - v_norm.y * s;
    (*out)(2, 1) = v_norm.z * v_norm.y * (1.0f - c) + v_norm.x * s;
    (*out)(2, 2) = c + v_norm.z * v_norm.z * (1.0f - c);
    (*out)(2, 3) = 0.0f;

    (*out)(3, 0) = 0.0f;
    (*out)(3, 1) = 0.0f;
    (*out)(3, 2) = 0.0f;
    (*out)(3, 3) = 1.0f;

    return out;
}

D3DXMATRIX* D3DXMatrixOrthoOffCenterRH(D3DXMATRIX* out,
                                       float l,
                                       float r,
                                       float b,
                                       float t,
                                       float zn,
                                       float zf) {

    if (!out) {
        return out;
    }

    if ((r == l) || (t == b) || (zn == zf)) {
        return out;
    }

    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterrh

    (*out)(0, 0) = 2.0f / (r - l);
    (*out)(0, 1) = 0.0f;
    (*out)(0, 2) = 0.0f;
    (*out)(0, 3) = 0.0f;

    (*out)(1, 0) = 0.0f;
    (*out)(1, 1) = 2.0f / (t - b);
    (*out)(1, 2) = 0.0f;
    (*out)(1, 3) = 0.0f;

    (*out)(2, 0) = 0.0f;
    (*out)(2, 1) = 0.0f;
    (*out)(2, 2) = 1.0f / (zn - zf);
    (*out)(2, 3) = 0.0f;

    (*out)(3, 0) = (l + r) / (l - r);
    (*out)(3, 1) = (t + b) / (b - t);
    (*out)(3, 2) = zn / (zn - zf);
    (*out)(3, 3) = 1.0f;


    return out;
}

D3DXMATRIX* D3DXMatrixOrthoOffCenterLH(D3DXMATRIX* out,
                                       float l,
                                       float r,
                                       float b,
                                       float t,
                                       float zn,
                                       float zf) {

    if (!out) {
        return out;
    }

    if ((r == l) || (t == b) || (zn == zf)) {
        return out;
    }

    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterlh

    (*out)(0, 0) = 2.0f / (r - l);
    (*out)(0, 1) = 0.0f;
    (*out)(0, 2) = 0.0f;
    (*out)(0, 3) = 0.0f;

    (*out)(1, 0) = 0.0f;
    (*out)(1, 1) = 2.0f / (t - b);
    (*out)(1, 2) = 0.0f;
    (*out)(1, 3) = 0.0f;

    (*out)(2, 0) = 0.0f;
    (*out)(2, 1) = 0.0f;
    (*out)(2, 2) = 1.0f / (zf - zn);
    (*out)(2, 3) = 0.0f;

    (*out)(3, 0) = (l + r) / (l - r);
    (*out)(3, 1) = (t + b) / (b - t);
    (*out)(3, 2) = zn / (zn - zf);
    (*out)(3, 3) = 1.0f;


    return out;
}

D3DXMATRIX* D3DXMatrixLookAtRH(D3DXMATRIX* out,
                               const D3DXVECTOR3* eye,
                               const D3DXVECTOR3* at,
                               const D3DXVECTOR3* up) {

    if (!out || !eye || !at || !up) {
        return out;
    }

    auto xmEye = toXmVector(*eye);
    auto xmAt = toXmVector(*at);
    auto xmUp = toXmVector(*up);

    auto xmOut = DirectX::XMMatrixLookAtRH(xmEye, xmAt, xmUp);

    toD3DXMatrix(*out, xmOut);
    return out;
}



D3DXMATRIX* D3DXMatrixAffineTransformation2D(D3DXMATRIX* out,
                                             float scaling,
                                             const D3DXVECTOR2* rotationCenter,
                                             float rotation,
                                             const D3DXVECTOR2* translation) {

    if (!out) {
        return out;
    }

    DirectX::XMVECTOR xmScaling {scaling, scaling, 0.0f, 0.0f};

    DirectX::XMVECTOR xmRotationCenter {
        rotationCenter ? rotationCenter->x : 0.0f,
        rotationCenter ? rotationCenter->y : 0.0f,
        0.0f,
        0.0f
    };

    DirectX::XMVECTOR xmTranslation {
        translation ? translation->x : 0.0f,
        translation ? translation->y : 0.0f,
        0.0f,
        0.0f
    };

    auto xmOut = DirectX::XMMatrixAffineTransformation2D
        (xmScaling,
         xmRotationCenter,
         rotation,
         xmTranslation);

    toD3DXMatrix(*out, xmOut);
    return out;
}

D3DXMATRIX* D3DXMatrixTransformation2D(D3DXMATRIX* out,
                                       const D3DXVECTOR2* scalingCenter,
                                       float scalingRotation,
                                       const D3DXVECTOR2* scaling,
                                       const D3DXVECTOR2* rotationCenter,
                                       float rotation,
                                       const D3DXVECTOR2* translation) {

    if (!out) {
        return out;
    }

    DirectX::XMVECTOR xmScalingCenter {
        scalingCenter ? scalingCenter->x : 0.0f,
        scalingCenter ? scalingCenter->y : 0.0f,
        0.0f,
        0.0f
    };

    DirectX::XMVECTOR xmScaling {
        scaling ? scaling->x : 0.0f,
        scaling ? scaling->y : 0.0f,
        0.0f,
        0.0f
    };

    DirectX::XMVECTOR xmRotationCenter {
        rotationCenter ? rotationCenter->x : 0.0f,
        rotationCenter ? rotationCenter->y : 0.0f,
        0.0f,
        0.0f
    };

    DirectX::XMVECTOR xmTranslation {
        translation ? translation->x : 0.0f,
        translation ? translation->y : 0.0f,
        0.0f,
        0.0f
    };

    auto xmOut = DirectX::XMMatrixTransformation2D
        (xmScalingCenter,
         scalingRotation,
         xmScaling,
         xmRotationCenter,
         rotation,
         xmTranslation);

    toD3DXMatrix(*out, xmOut);
    return out;
}


D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* out,
                               const D3DXMATRIX* a,
                               const D3DXMATRIX* b) {

    if (!out || !a || !b) {
        return out;
    }

    auto xm_a = toXmMatrix(*a);
    auto xm_b = toXmMatrix(*b);
    auto xm_out = DirectX::XMMatrixMultiply(xm_a, xm_b);

    toD3DXMatrix(*out, xm_out);
    return out;
}

D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* out,
                              float sx,
                              float sy,
                              float sz) {


    if (!out) {
        return out;
    }

    (*out)(0, 0) = sx;
    (*out)(0, 1) = 0.0f;
    (*out)(0, 2) = 0.0f;
    (*out)(0, 3) = 0.0f;

    (*out)(1, 0) = 0.0f;
    (*out)(1, 1) = sy;
    (*out)(1, 2) = 0.0f;
    (*out)(1, 3) = 0.0f;

    (*out)(2, 0) = 0.0f;
    (*out)(2, 1) = 0.0f;
    (*out)(2, 2) = sz;
    (*out)(2, 3) = 0.0f;

    (*out)(3, 0) = 0.0f;
    (*out)(3, 1) = 0.0f;
    (*out)(3, 2) = 0.0f;
    (*out)(3, 3) = 1.0f;

    return out;
}



D3DXVECTOR4* D3DXVec4Transform(D3DXVECTOR4* out,
                               const D3DXVECTOR4* v,
                               const D3DXMATRIX* m) {

    if (!v || !m || !out) {
        return out;
    }

    out->x = v->x * (*m)(0, 0) +
             v->y * (*m)(0, 1) +
             v->z * (*m)(0, 2) +
             v->w * (*m)(0, 3);

    out->y = v->x * (*m)(1, 0) +
             v->y * (*m)(1, 1) +
             v->z * (*m)(1, 2) +
             v->w * (*m)(1, 3);

    out->z = v->x * (*m)(2, 0) +
             v->y * (*m)(2, 1) +
             v->z * (*m)(2, 2) +
             v->w * (*m)(2, 3);

    out->w = v->x * (*m)(3, 0) +
             v->y * (*m)(3, 1) +
             v->z * (*m)(3, 2) +
             v->w * (*m)(3, 3);

    return out;
}

D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* out,
                               const D3DXVECTOR3* in) {

    if (!out || !in) {
        return out;
    }

    auto norm =
        sqrtf(in->x * in->x + in->y * in->y + in->z * in->z);

    *out *= norm;
    return out;
}

D3DXVECTOR4* D3DXVec3Transform(D3DXVECTOR4* out,
                               const D3DXVECTOR3* v,
                               const D3DXMATRIX* m) {

    if (!v || !m || !out) {
        return out;
    }

    D3DXVECTOR4 v4 {};
    v4.x = v->x;
    v4.y = v->y;
    v4.z = v->z;
    v4.w = 1.0f;

    return D3DXVec4Transform(out, &v4, m);
}

D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* out,
                                    const D3DXVECTOR3* v,
                                    const D3DXMATRIX* m) {

    if (!v || !m || !out) {
        return out;
    }

    D3DXVECTOR4 v4 {};
    v4.x = v->x;
    v4.y = v->y;
    v4.z = v->z;
    v4.w = 1.0f;
    D3DXVECTOR4 out4 {};

    D3DXVec4Transform(&out4, &v4, m);
    if (out4.w != 0.0f) {
        out4 /= out4.w;
    }

    out->x = out4.x;
    out->y = out4.y;
    out->z = out4.z;
    return out;
}

D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* out,
                                     const D3DXVECTOR3* v,
                                     const D3DXMATRIX* m) {

    if (!v || !m || !out) {
        return out;
    }

    D3DXVECTOR4 v4 {};
    v4.x = v->x;
    v4.y = v->y;
    v4.z = v->z;
    v4.w = 0.0f;
    D3DXVECTOR4 out4 {};

    D3DXVec4Transform(&out4, &v4, m);
    // TODO(jec):  Drop w from out4?

    out->x = out4.x;
    out->y = out4.y;
    out->z = out4.z;
    return out;
}



float vec3dot(const D3DXVECTOR3& a,
              const D3DXVECTOR3& b) {

    return a.x * b.x + a.y * b.y + a.z * b.z;
}

int D3DXIntersectTri(const D3DXVECTOR3* p0,
                     const D3DXVECTOR3* p1,
                     const D3DXVECTOR3* p2,
                     const D3DXVECTOR3* pRayPos,
                     const D3DXVECTOR3* pRayDir,
                     float* pU,
                     float* pV,
                     float* pDist) {


    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

    if (!p0 || !p1 || !p2 || !pRayPos || !pRayDir) {
        return 0;
    }

    auto e1 = *p1 - *p0;
    auto e2 = *p2 - *p0;

    D3DXVECTOR3 ray_cross_e2 {};
    D3DXVec3Cross(&ray_cross_e2, pRayDir, &e2);

    auto dot_p = vec3dot(e1, ray_cross_e2);

    if (std::abs(dot_p) < 1.0e-6f) {
        return 0; // Ray does not intersect triangle.
    }

    auto inv_dot_p = 1.0f / dot_p;
    D3DXVECTOR3 s = *pRayPos - *p0;
    auto u = inv_dot_p * vec3dot(s, ray_cross_e2);

    D3DXVECTOR3 s_cross_e1 {};
    D3DXVec3Cross(&s_cross_e1, &s, &e1);
    auto v = inv_dot_p * vec3dot(*pRayDir, s_cross_e1);

    auto t = inv_dot_p * vec3dot(e2, s_cross_e1);

    if (pU) {
        *pU = u;
    }
    if (pV) {
        *pV = v;
    }
    if (pDist) {
        *pDist = t;
    }

    return (u >= 0.0f) &&
           (u <= 1.0f) &&
           (v >= 0.0f) &&
           (u + v <= 1.0f) &&
           (t > 1.0e-6f);
}


