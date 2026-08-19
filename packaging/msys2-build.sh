#!/bin/bash
#
# Builds the Windows installer from an MSYS2 shell. Works in both the
# UCRT64 (x64) and CLANGARM64 (ARM64) environments.

packages=(
        cc
        cmake
        enet
        physfs
        curl-winssl
        SDL2_image
        SDL2_mixer
        SDL2_net
        SDL2_ttf
        libxml2
        gettext
)

# NSIS is not packaged for CLANGARM64, install it from https://nsis.sourceforge.io/ instead
if [ "$MSYSTEM" != "CLANGARM64" ]; then
        packages+=(nsis)
fi

pacman --noconfirm -S "${packages[@]/#/${MINGW_PACKAGE_PREFIX}-}"

cmake -B build . -DUSE_SYSTEM_GUICHAN=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build
cpack --config build/CPackConfig.cmake
