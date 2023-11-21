#!/bin/bash
cmake \
    -DCMAKE_TOOLCHAIN_FILE=../clang-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DORBITER_DXVK=1 \
    -DDXVK_DIR=~/src/dxvk/package/dxvk-native-2.3.9/usr \
    -DORBITER_SDL_INPUT=ON \
    -DORBITER_BUILD_GDICLIENT=OFF \
    -DCMAKE_INSTALL_PREFIX=~/orbiter-linux-dxvk \
    ..
