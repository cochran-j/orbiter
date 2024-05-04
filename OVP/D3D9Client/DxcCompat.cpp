/* DxcCompat.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

/* WARNING(jec):  There's an #include order problem here, probably because
 * of some kind of bizarre unterminated definition in DxcCompat.h.
 */
#include <istream>

#include "DxcCompat.h"

#include <sstream>
#include <string>

namespace d3d9client {

StringDxcBuffer::StringDxcBuffer()
    : buf{},
      dxbuf{} {

    updateDxBuf();
}

StringDxcBuffer::StringDxcBuffer(std::istream& input_stream)
    : buf{},
      dxbuf{} {

    /* TODO(jec):  Unclear from docs what encoding is supposed to be, just
     * 0 for "ANSI" or "unknown with BOM".  Yay, MSFT!
     */
    dxbuf.Encoding = 0;

    std::ostringstream os {};
    os << input_stream.rdbuf();
    buf = os.str();


    updateDxBuf();
}


void StringDxcBuffer::updateDxBuf() {
    dxbuf.Ptr = buf.data();
    dxbuf.Size = buf.size();
}

const struct DxcBuffer& StringDxcBuffer::dxcBuffer() const {
    return dxbuf;
}


D3DXBuffer_Unknown_helper::D3DXBuffer_Unknown_helper()
    : ref_count{1} {

    return;
}

ULONG D3DXBuffer_Unknown_helper::AddRef() {
    return ++ref_count;
}

HRESULT D3DXBuffer_Unknown_helper::QueryInterface
    (REFIID riid, void** ppvObject) {

    if (!ppvObject) {
        return E_POINTER;
    }

    if (riid == uuidof(IUnknown)) {
        *ppvObject = static_cast<IUnknown*>(this);
        return S_OK;
    }

    if (riid == uuidof(ID3DXBuffer)) {
        *ppvObject = static_cast<ID3DXBuffer*>(this);
        return S_OK;
    }

    return E_NOINTERFACE;
}

ULONG D3DXBuffer_Unknown_helper::Release() {
    auto new_count = --ref_count;
    if (new_count == 0) {
        delete this;
    }
    return new_count;
}


DxcBlob_D3DXBuffer::DxcBlob_D3DXBuffer()
    : D3DXBuffer_Unknown_helper{},
      myblob{nullptr} {

    return;
}

DxcBlob_D3DXBuffer::DxcBlob_D3DXBuffer(IDxcBlob* in_blob)
    : D3DXBuffer_Unknown_helper{},
      myblob{in_blob} {

    if (myblob) {
        myblob->AddRef();
    }
}

ULONG DxcBlob_D3DXBuffer::Release() {
    if ((ref_count == 1) && myblob) {

        myblob->Release();
    }
    D3DXBuffer_Unknown_helper::Release();
}

LPVOID DxcBlob_D3DXBuffer::GetBufferPointer() {
    return myblob ? myblob->GetBufferPointer() : nullptr;
}

DWORD DxcBlob_D3DXBuffer::GetBufferSize() {
    return myblob ? static_cast<DWORD>(myblob->GetBufferSize()) : 0;
}



DxcBlobUtf8_D3DXBuffer::DxcBlobUtf8_D3DXBuffer()
    : D3DXBuffer_Unknown_helper{},
      myblob{nullptr} {

    return;
}

DxcBlobUtf8_D3DXBuffer::DxcBlobUtf8_D3DXBuffer(IDxcBlobUtf8* in_blob)
    : D3DXBuffer_Unknown_helper{},
      myblob{in_blob} {

    if (myblob) {
        myblob->AddRef();
    }
}

ULONG DxcBlobUtf8_D3DXBuffer::Release() {
    if ((ref_count == 1) && myblob) {
        myblob->Release();
    }
    D3DXBuffer_Unknown_helper::Release();
}

LPVOID DxcBlobUtf8_D3DXBuffer::GetBufferPointer() {
    return myblob ? const_cast<char*>(myblob->GetStringPointer()) : nullptr;
}

DWORD DxcBlobUtf8_D3DXBuffer::GetBufferSize() {
    return myblob ? static_cast<DWORD>(myblob->GetStringLength()) : 0;
}



}

