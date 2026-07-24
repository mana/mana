#!/bin/bash -e
#
# Packages an existing build as an AppImage using linuxdeploy. The build
# directory (first argument, defaults to "build") must have been configured
# with -DCMAKE_INSTALL_PREFIX=/usr.
#
# Requires file and wget in addition to the build dependencies.

BUILD_DIR=${1:-build}
APPDIR=$PWD/AppDir

rm -rf "$APPDIR"
DESTDIR=$APPDIR cmake --install "$BUILD_DIR"

if [ ! -x linuxdeploy-x86_64.AppImage ]; then
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

# Allows running AppImages inside containers, where FUSE is unavailable
export APPIMAGE_EXTRACT_AND_RUN=1

# No version in the file name, so that the latest artifact has a stable
# download URL usable on the website (the client reports its version with
# --version)
./linuxdeploy-x86_64.AppImage \
    --appdir "$APPDIR" \
    --desktop-file "$APPDIR/usr/share/applications/org.manasource.Mana.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/scalable/apps/org.manasource.Mana.svg" \
    --output appimage

# Smoke test (exits before initializing any video/audio)
./Mana-*.AppImage --version
