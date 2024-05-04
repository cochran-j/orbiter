/* EmulateUUID.h
 * Fake some stuff that is provided by the MSVC compiler.
 *
 * Taken from DirectXShaderCompiler:  dxc/WinAdapter.h.  We are copying this
 * out here, in order to avoid having to #include WinAdapter.h, which is full
 * of basic Win32 typedefs that are incompatible with the dxvk typedefs.
 *
 * Code from:
 *
 * The LLVM Compiler Infrastructure
 *
 * This file is distributed under the University of Illinois Open Source
 * License.  See LICENSE.TXT for details.
 *
 */

#ifndef OVP_D3D9CLIENT_EMULATEUUID_H__
#define OVP_D3D9CLIENT_EMULATEUUID_H__

#include <stdint.h>
#include <type_traits>

// The following macros are defined to facilitate the lack of 'uuid' on Linux.

constexpr uint8_t nybble_from_hex(char c) {
    return ((c >= '0' && c <= '9')
                ? (c - '0')
                : ((c >= 'a' && c <= 'f')
                    ? (c - 'a' + 10)
                    : ((c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                              : /* Should be an error */ -1)));
}

constexpr uint8_t byte_from_hex(char c1, char c2) {
    return nybble_from_hex(c1) << 4 | nybble_from_hex(c2);
}

constexpr uint8_t byte_from_hexstr(const char str[2]) {
    return nybble_from_hex(str[0]) << 4 | nybble_from_hex(str[1]);
}

constexpr GUID guid_from_string(const char str[37]) {
    return GUID{static_cast<uint32_t>(byte_from_hexstr(str)) << 24 |
                    static_cast<uint32_t>(byte_from_hexstr(str + 2)) << 16 |
                    static_cast<uint32_t>(byte_from_hexstr(str + 4)) << 8 |
                    byte_from_hexstr(str + 6),
                static_cast<uint16_t>(
                    static_cast<uint16_t>(byte_from_hexstr(str + 9)) << 8 |
                    byte_from_hexstr(str + 11)),
                static_cast<uint16_t>(
                    static_cast<uint16_t>(byte_from_hexstr(str + 14)) << 8 |
                    byte_from_hexstr(str + 16)),
                {byte_from_hexstr(str + 19), byte_from_hexstr(str + 21),
                 byte_from_hexstr(str + 24), byte_from_hexstr(str + 26),
                 byte_from_hexstr(str + 28), byte_from_hexstr(str + 30),
                 byte_from_hexstr(str + 32), byte_from_hexstr(str + 34)}};
}

template <typename iface> inline GUID emulated_uuidof();

#define CROSS_PLATFORM_UUIDOF(iface, spec) \
    struct iface; \
    template <> inline GUID emulated_uuidof<iface>() { \
      static const IID _IID = guid_from_string(spec); \
      return _IID; \
    }

#define uuidof(T) emulated_uuidof<typename std::decay<T>::type>()



#endif /* OVP_D3D9CLIENT_EMULATEUUID_H__ */

