/* DxcCompat.h
 * Compatibility layer to interact with DirectXShader compiler (dxc).
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef OVP_D3D9CLIENT_DXCCOMPAT_H__
#define OVP_D3D9CLIENT_DXCCOMPAT_H__

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
#include "d3dx9mesh.h"
#include "sal.h" /* Several _In_ / _Out_ definitions */

/* Try to fake dxcapi.h from trying to provide its own common win32 and COM
 * definitions that clash with the dxvk definitions.
 */
#include "EmulateUUID.h"
#define DXC_API_NO_WIN_ADAPTER
using BSTR = char16_t*;
using CLSID = GUID;
#include "dxc/dxcapi.h"

#include <istream>
#include <string>
#include <cstddef>

namespace d3d9client {


class StringDxcBuffer {
public:
    StringDxcBuffer();
    StringDxcBuffer(std::istream& input_stream);

    const struct DxcBuffer& dxcBuffer() const;

    /* TODO(jec):  add string methods as necessary. */

private:
    void updateDxBuf();

private:
    std::string buf;
    struct DxcBuffer dxbuf;

};

class D3DXBuffer_Unknown_helper : public ID3DXBuffer {
public:
    D3DXBuffer_Unknown_helper ();
    virtual ~D3DXBuffer_Unknown_helper() {  }

    /* IUnknown */
    virtual ULONG AddRef() override;
    virtual HRESULT QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG Release() override;

    /*
    LPVOID GetBufferPointer() {
        return buffer.data();
    }

    DWORD GetBufferSize() {
        return static_cast<DWORD>(buffer.size());
    }
    */

protected:
    std::size_t ref_count;
};

class DxcBlob_D3DXBuffer : public D3DXBuffer_Unknown_helper {
public:
    DxcBlob_D3DXBuffer();
    DxcBlob_D3DXBuffer(IDxcBlob* in_blob);

    /* IUnknown */
    ULONG Release() override;

    /* ID3DXBuffer */
    LPVOID GetBufferPointer() override;
    DWORD GetBufferSize() override;


private:
    IDxcBlob* myblob;
};

class DxcBlobUtf8_D3DXBuffer : public D3DXBuffer_Unknown_helper {
public:
    DxcBlobUtf8_D3DXBuffer();
    DxcBlobUtf8_D3DXBuffer(IDxcBlobUtf8* in_blob);

    /* IUnknown */
    ULONG Release() override;

    /* ID3DXBuffer */
    LPVOID GetBufferPointer() override;
    DWORD GetBufferSize() override;

private:
    IDxcBlobUtf8* myblob;
};



class D3DXConstantTable : public ID3DXConstantTable {
public:
    D3DXConstantTable();

    /* IUnknown */
    ULONG AddRef() override;
    HRESULT QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG Release() override;

    /* ID3DXConstantTable */
    /* Gets a pointer to the buffer that contains the constant table. */
    LPVOID GetBufferPointer() override;

    /* Gets the buffer size of the constant table. */
    DWORD GetBufferSize() override;

    /* Gets a constant by looking up its index. */
    D3DXHANDLE GetConstant(D3DXHANDLE hConstant, UINT index) override;

    /* Gets a constant by looking up its name. */
    D3DXHANDLE GetConstantByName(D3DXHANDLE hConstant, const char* name) override;

    /* Gets a pointer to an array of constant descriptions in the constant
     * table
     */
    HRESULT GetConstantDesc(D3DXHANDLE hConstant,
                            D3DXCONSTANT_DESC* pDesc,
                            UINT* pCount) override;

    /* Gets a constant from an array of constants.  An array is made up of
     * elements.
     */
    D3DXHANDLE GetConstantElement(D3DXHANDLE hConstant, UINT index) override;

    /* Gets a description of the constant table. */
    HRESULT GetDesc(D3DXCONSTANTTABLE_DESC* pDesc) override;

    /* Returns the sampler index. */
    UINT GetSamplerIndex(D3DXHANDLE hConstant) override;

    /* Sets a Boolean value. */
    HRESULT SetBool(struct IDirect3DDevice9* pDevice,
                    D3DXHANDLE hConstant,
                    BOOL b) override;

    /* Sets an array of Boolean values. */
    HRESULT SetBoolArray(struct IDirect3DDevice9* pDevice,
                         D3DXHANDLE hConstant,
                         const BOOL* pB,
                         UINT Count) override;

    /* Sets the constants to their default values.  The default values are
     * declared in the variable declarations in the shader.
     */
    HRESULT SetDefaults(struct IDirect3DDevice9* pDevice) override;

    /* Sets a floating-point number. */
    HRESULT SetFloat(struct IDirect3DDevice9* pDevice,
                     D3DXHANDLE hConstant,
                     float f) override;

    /* Sets an array of floating-point numbers. */
    HRESULT SetFloatArray(struct IDirect3DDevice9* pDevice,
                          D3DXHANDLE hConstant,
                          const float* pF,
                          UINT Count) override;

    /* Sets an integer value. */
    HRESULT SetInt(struct IDirect3DDevice9* pDevice,
                   D3DXHANDLE hConstant,
                   int n) override;

    /* Sets an array of integers. */
    HRESULT SetIntArray(struct IDirect3DDevice9* pDevice,
                        D3DXHANDLE hConstant,
                        const int* pn,
                        UINT Count) override;

    /* Sets a nontransposed matrix. */
    HRESULT SetMatrix(struct IDirect3DDevice9* pDevice,
                      D3DXHANDLE hConstant,
                      const D3DXMATRIX* pMatrix) override;

    /* Sets an array of nontransposed matrices. */
    HRESULT SetMatrixArray(struct IDirect3DDevice9* pDevice,
                           D3DXHANDLE hConstant,
                           const D3DXMATRIX* pMatrix,
                           UINT Count) override;

    /* Sets an array of pointers to nontransposed matrices. */
    HRESULT SetMatrixPointerArray(struct IDirect3DDevice9* pDevice,
                                  D3DXHANDLE hConstant,
                                  const D3DXMATRIX** ppMatrix,
                                  UINT Count) override;

    /* Sets a transposed matrix. */
    HRESULT SetMatrixTranspose(struct IDirect3DDevice9* pDevice,
                               D3DXHANDLE hConstant,
                               const D3DXMATRIX* pMatrix) override;

    /* Sets an array of transposed matrices. */
    HRESULT SetMatrixTransposeArray(struct IDirect3DDevice9* pDevice,
                                    D3DXHANDLE hConstant,
                                    const D3DXMATRIX* pMatrix,
                                    UINT Count) override;

    /* Sets an array of pointers to transposed matrices. */
    HRESULT SetMatrixTransposePointerArray(struct IDirect3DDevice9* pDevice,
                                           D3DXHANDLE hConstant,
                                           const D3DXMATRIX** ppMatrix,
                                           UINT Count) override;


    /* Sets the contents of the buffer to the constant table. */
    HRESULT SetValue(struct IDirect3DDevice9* pDevice,
                     D3DXHANDLE hConstant,
                     const void* pData,
                     UINT Bytes) override;

    /* Sets a 4D vector. */
    HRESULT SetVector(struct IDirect3DDevice9* pDevice,
                      D3DXHANDLE hConstant,
                      const D3DXVECTOR4* pVector) override;


    /* Sets an array of 4D vectors. */
    HRESULT SetVectorArray(struct IDirect3DDevice9* pDevice,
                           D3DXHANDLE hConstant,
                           const D3DXVECTOR4* pVector,
                           UINT Count) override;
};



}

#endif /* OVP_D3D9CLIENT_DXCCOMPAT_H__ */

