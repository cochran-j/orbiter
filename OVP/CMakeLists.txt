# Copyright (c) Martin Schweiger
# Licensed under the MIT License

if(ORBITER_BUILD_GDICLIENT)
    add_subdirectory(GDIClient)
endif()

# TODO(jec): BUILD64 unset ORBITER_BUILD option.
if(ORBITER_BUILD_D3D7CLIENT AND NOT BUILD64)
	add_subdirectory(D3D7Client)
endif()

if(ORBITER_BUILD_D3D9CLIENT)
	add_subdirectory(D3D9Client)
endif()
