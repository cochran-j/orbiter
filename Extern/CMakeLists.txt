project(Extern)

##########################################
## Lua

file(GLOB LUA_FILES "Lua/*.c" "Lua/*.h")

add_library(lua SHARED ${LUA_FILES})
if(WIN32)
    target_compile_definitions(lua PUBLIC LUA_BUILD_AS_DLL)
endif()

# for tests and running Orbiter from inside the build dir
set_target_properties(lua PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${ORBITER_BINARY_ROOT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${ORBITER_BINARY_ROOT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_ROOT_DIR}
)

install(TARGETS
    lua
    RUNTIME DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
    LIBRARY DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
)
install(TARGETS lua DESTINATION ${ORBITER_INSTALL_SDK_DIR}/lib/Lua)

##########################################
## zlib

set(SKIP_INSTALL_ALL ON)
add_subdirectory(zlib)

# Set option for POSIX compatibility to get fileno() in
# stdio.h
if(UNIX)
    target_compile_definitions(zlib PUBLIC _POSIX_SOURCE)
endif()

target_include_directories(zlib INTERFACE ${zlib_SOURCE_DIR})
target_include_directories(zlib INTERFACE ${zlib_BINARY_DIR}) # for zconf.h

# for tests and running Orbiter from inside the build dir
set_target_properties(zlib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${ORBITER_BINARY_ROOT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${ORBITER_BINARY_ROOT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_ROOT_DIR}
)

install(TARGETS zlib
    RUNTIME DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
    LIBRARY DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
)

##########################################
## Catch2

if(ORBITER_MAKE_TESTS)
    add_subdirectory(Catch2)
endif()


#########################################
## DirectXTK and DirectXTex.
## Designated replacements for D3DX.
if(ORBITER_BUILD_D3D9CLIENT)
    add_subdirectory(DirectXMath)

    set(DIRECTXMATH_INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/Extern/DirectXMath/Inc
    )
    # WARN(jec):  DirectXTK is not very portable.
    # DirectXTK shader build uses cmd batch script.
    #set(USE_PREBUILT_SHADERS ON CACHE BOOL
    #"build shaders separately"
    #FORCE)
    #add_subdirectory(DirectXTK)
    #target_include_directories(DirectXTK PRIVATE
    #${WIN32_INCLUDE_DIR}
    #${DXVK_INCLUDE_DIR}/directx
    #${CMAKE_SOURCE_DIR}/Extern/DirectXMath/Inc
    #${CMAKE_SOURCE_DIR}/Extern/SAL
    #)
endif()
