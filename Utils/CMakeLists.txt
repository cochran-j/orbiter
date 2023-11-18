# Copyright (c) Martin Schweiger
# Licensed under the MIT License

include(ExternalProject)

add_subdirectory(Date)
add_subdirectory(meshc)
if(WIN32) # TODO(jec):  Pltex not very portable
    add_subdirectory(Pltex)
endif()
add_subdirectory(Shipedit)
if(WIN32) # TODO(jec):  texpack is actually fairly portable.
add_subdirectory(texpack)
endif()
#add_subdirectory(tileedit/qt)

if(WIN32) # TODO(jec):  plsplit not very portable.
    # We do this as an external project to invoke x64 toolchain
    ExternalProject_Add(plsplit
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/plsplit
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/plsplit
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/plsplit
        INSTALL_DIR ${ORBITER_INSTALL_UTILS_DIR}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DINSTALLDIR=${ORBITER_INSTALL_UTILS_DIR}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/plsplit --config Release
        INSTALL_COMMAND ""
    )
    # Install project output from here instead from inside the external project
    # to make cpack aware of it
    if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/plsplit/plsplit.exe)
        install(PROGRAMS
            ${CMAKE_CURRENT_BINARY_DIR}/plsplit/plsplit.exe
            plsplit/DxTex.exe
            DESTINATION ${ORBITER_INSTALL_UTILS_DIR}
        )
    endif()
endif()

# tileedit is another 64-bit project. This also requires
# Qt5 so will only be built if that is available
ExternalProject_Add(tileedit_proj
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/tileedit/qt
	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/tileedit/qt
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tileedit/qt
	INSTALL_DIR ${ORBITER_INSTALL_UTILS_DIR}
	CMAKE_ARGS -DCMAKE_CFG_INTDIR=Release -DQTDIR:PATH=${Qt5_x64_DIR}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DINSTALLDIR=${ORBITER_INSTALL_UTILS_DIR}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
	BUILD_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/tileedit/qt --config Release
	INSTALL_COMMAND ""
)
# Install project output from here instead from inside the external project
# to make cpack aware of it
if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/tileedit/qt/src/tmp)
	install(DIRECTORY
		${CMAKE_CURRENT_BINARY_DIR}/tileedit/qt/src/tmp/
		DESTINATION ${ORBITER_INSTALL_UTILS_DIR}
	)
endif()
