# Copyright (c) Martin Schweiger
# Licensed under the MIT License

add_executable(Orbiter_ng
	Orbiter_ng.cpp
	Orbiter_ng.rc
)

set_target_properties(Orbiter_ng
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	FOLDER Core
)

if(MSVC)
    set_target_properties(Orbiter_ng
        PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    )
endif()

target_include_directories(Orbiter_ng
    PUBLIC ${WIN32_INCLUDE_DIR}
)

#Installation
install(TARGETS
	Orbiter_ng
	RUNTIME
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
	)
