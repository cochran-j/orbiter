find_path(DXVK_DIR
	include/dxvk/directx/d3dx9.h
	HINTS "/usr/local"
    PATHS ENV DXVK_DIR
)

set(DXVK_INCLUDE_DIR ${DXVK_DIR}/include/dxvk)
# TODO:  32-bit support
set(DXVK_LIB_DIR ${DXVK_DIR}/lib)

if(${DXVK_DIR} STREQUAL "DXVK_DIR-NOTFOUND")
    set(DXVK_FOUND FALSE)
else()
    set(DXVK_FOUND TRUE)
endif()
