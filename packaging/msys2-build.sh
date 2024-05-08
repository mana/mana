#!/bin/bash

pacman --noconfirm -S \
        mingw-w64-ucrt-x86_64-gcc \
        mingw-w64-ucrt-x86_64-cmake \
        mingw-w64-ucrt-x86_64-physfs \
        mingw-w64-ucrt-x86_64-curl-winssl \
        mingw-w64-ucrt-x86_64-SDL2_image \
        mingw-w64-ucrt-x86_64-SDL2_mixer \
        mingw-w64-ucrt-x86_64-SDL2_net \
        mingw-w64-ucrt-x86_64-SDL2_ttf \
        mingw-w64-ucrt-x86_64-libxml2 \
        mingw-w64-ucrt-x86_64-nsis

cmake -B build . -DUSE_SYSTEM_GUICHAN=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build
pushd build
cpack
