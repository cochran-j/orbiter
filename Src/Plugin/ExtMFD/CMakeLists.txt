# Copyright (c) Martin Schweiger
# Licensed under the MIT License

add_library(ExtMFD SHARED
	ExtMFD.cpp
	ExtMFD.rc
	MFDWindow.cpp
)

target_include_directories(ExtMFD
	PUBLIC ${ORBITER_SOURCE_SDK_INCLUDE_DIR}
	PUBLIC ${MODULE_COMMON_DIR}
    PUBLIC ${WIN32_INCLUDE_DIR}
)

target_link_libraries(ExtMFD
    # TODO(jec)
    # ${ORBITER_LIB}
	${ORBITER_SDK_LIB}
)

add_dependencies(ExtMFD
	${OrbiterTgt}
	Orbitersdk
)

set_target_properties(ExtMFD
	PROPERTIES
	FOLDER Modules
)

# Installation
install(TARGETS ExtMFD
	RUNTIME
	DESTINATION ${ORBITER_INSTALL_PLUGIN_DIR}
    LIBRARY
    DESTINATION ${ORBITER_INSTALL_PLUGIN_DIR}
)
