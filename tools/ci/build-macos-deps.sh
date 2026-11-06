#!/usr/bin/env bash
#
# Build the non-system dependencies for the macOS CI legs into a private
# prefix. Homebrew is intentionally not used: the Intel runner is a
# Homebrew Tier 3 configuration without bottles, so even a simple
# "brew install sdl2_image" builds an enormous source dependency tree and
# hits the job timeout.
#
# Everything is either built from source here or taken from the macOS SDK
# (curl, libxml2, zlib, iconv). The only downloaded binary is the
# official SDL3 dmg, from which the framework binary is thinned to the
# target architecture and kept as libSDL3.dylib. sdl2-compat loads it
# through its @loader_path/libSDL3.dylib fallback once bundled.
#
# Environment:
#   DEPS_PREFIX               install prefix (default: $PWD/deps)
#   ARCH                      arm64 or x86_64 (default: uname -m)
#   MACOSX_DEPLOYMENT_TARGET  minimum macOS version for the compiled deps
#
# The pinned versions below feed the actions/cache key through the hash
# of this file, so bumping a version automatically invalidates the cache.
#
# Implemented with Devin (SWE-2).

set -euo pipefail

SDL3_VERSION=3.4.20
SDL2_COMPAT_VERSION=2.32.74
SDL2_IMAGE_VERSION=2.8.12
SDL2_MIXER_VERSION=2.8.2
SDL2_NET_VERSION=2.4.0
SDL2_TTF_VERSION=2.24.0
PHYSFS_VERSION=3.2.0
LIBICONV_VERSION=1.19
GETTEXT_VERSION=0.26
PKGCONF_VERSION=3.0.10

DEPS_PREFIX="${DEPS_PREFIX:-$PWD/deps}"
ARCH="${ARCH:-$(uname -m)}"
export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-13.3}"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

STAMP="SDL3=$SDL3_VERSION sdl2-compat=$SDL2_COMPAT_VERSION SDL2_image=$SDL2_IMAGE_VERSION SDL2_mixer=$SDL2_MIXER_VERSION SDL2_net=$SDL2_NET_VERSION SDL2_ttf=$SDL2_TTF_VERSION physfs=$PHYSFS_VERSION libiconv=$LIBICONV_VERSION gettext=$GETTEXT_VERSION pkgconf=$PKGCONF_VERSION arch=$ARCH target=$MACOSX_DEPLOYMENT_TARGET"

if [ -f "$DEPS_PREFIX/.stamp" ] && [ "$(cat "$DEPS_PREFIX/.stamp")" = "$STAMP" ]; then
    echo "Dependencies already built (cache hit), skipping"
    exit 0
fi

# Rebuild from scratch: a prefix without a matching stamp is either
# absent, partial or stale
if [ -e "$DEPS_PREFIX" ]; then
    case "$DEPS_PREFIX" in
        ""|"/"|"$PWD"|"$PWD/"|*" "*)
            echo "Refusing to wipe suspicious DEPS_PREFIX: $DEPS_PREFIX" >&2
            exit 1 ;;
    esac
    rm -rf "$DEPS_PREFIX"
fi
mkdir -p "$DEPS_PREFIX"/{bin,include,lib,Frameworks}
DEPS_PREFIX="$(cd "$DEPS_PREFIX" && pwd)"

BUILD_ROOT="$PWD/deps-build"
rm -rf "$BUILD_ROOT"
mkdir -p "$BUILD_ROOT"
cd "$BUILD_ROOT"

fetch() { # <file> <url>
    echo "==> Downloading $1"
    curl -fL --retry 3 -o "$1" "$2"
    tar -xf "$1"
}

build_cmake_dep() { # <srcdir> [cmake args...]
    local src="$1"
    shift
    cmake -S "$src" -B "$src/build" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOSX_DEPLOYMENT_TARGET" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
        -DCMAKE_PREFIX_PATH="$DEPS_PREFIX" \
        -DCMAKE_INSTALL_PREFIX="$DEPS_PREFIX" \
        "$@"
    cmake --build "$src/build" --parallel "$JOBS"
    cmake --install "$src/build"
}

# pkgconf, needed by the satellite libs to locate SDL2 and by Mana's own
# configure step; do not rely on a preinstalled pkg-config
fetch "pkgconf-$PKGCONF_VERSION.tar.xz" \
    "https://distfiles.ariadne.space/pkgconf/pkgconf-$PKGCONF_VERSION.tar.xz"
(
    cd "pkgconf-$PKGCONF_VERSION"
    ./configure --prefix="$DEPS_PREFIX" --disable-static
    make -j"$JOBS"
    make install
)
ln -sf pkgconf "$DEPS_PREFIX/bin/pkg-config"

export PATH="$DEPS_PREFIX/bin:$PATH"
export PKG_CONFIG_PATH="$DEPS_PREFIX/lib/pkgconfig"

# SDL3 (binary release). The dmg ships an XCFramework; the universal
# macOS SDL3.framework slice is thinned to lib/libSDL3.dylib and its
# headers go to include/SDL3.
echo "==> Downloading SDL3 $SDL3_VERSION (dmg)"
curl -fL --retry 3 -o "SDL3-$SDL3_VERSION.dmg" \
    "https://github.com/libsdl-org/SDL/releases/download/release-$SDL3_VERSION/SDL3-$SDL3_VERSION.dmg"
hdiutil attach -nobrowse -readonly -mountpoint "$BUILD_ROOT/sdl3-mnt" \
    "SDL3-$SDL3_VERSION.dmg"
cp -R "$BUILD_ROOT/sdl3-mnt/SDL3.xcframework/macos-arm64_x86_64/SDL3.framework" \
    "$DEPS_PREFIX/Frameworks/"
hdiutil detach "$BUILD_ROOT/sdl3-mnt" -quiet || \
    hdiutil detach "$BUILD_ROOT/sdl3-mnt" -force
lipo -thin "$ARCH" \
    "$DEPS_PREFIX/Frameworks/SDL3.framework/Versions/A/SDL3" \
    -output "$DEPS_PREFIX/lib/libSDL3.dylib"
cp -R "$DEPS_PREFIX/Frameworks/SDL3.framework/Versions/A/Headers" \
    "$DEPS_PREFIX/include/SDL3"

# sdl2-compat provides libSDL2.dylib plus sdl2.pc; it only needs the SDL3
# headers at build time (SDL3 itself is dlopen'ed at runtime)
fetch "sdl2-compat-$SDL2_COMPAT_VERSION.tar.gz" \
    "https://github.com/libsdl-org/sdl2-compat/releases/download/release-$SDL2_COMPAT_VERSION/sdl2-compat-$SDL2_COMPAT_VERSION.tar.gz"
build_cmake_dep "sdl2-compat-$SDL2_COMPAT_VERSION" \
    -DSDL2COMPAT_TESTS=OFF \
    -DSDL2COMPAT_X11=OFF \
    -DSDL3_INCLUDE_DIRS="$DEPS_PREFIX/include"

# SDL2_image: the built-in stb_image and ImageIO backends cover PNG and
# JPEG, so no external image libraries are needed
fetch "SDL2_image-$SDL2_IMAGE_VERSION.tar.gz" \
    "https://github.com/libsdl-org/SDL_image/releases/download/release-$SDL2_IMAGE_VERSION/SDL2_image-$SDL2_IMAGE_VERSION.tar.gz"
build_cmake_dep "SDL2_image-$SDL2_IMAGE_VERSION" \
    -DSDL2IMAGE_VENDORED=ON \
    -DSDL2IMAGE_SAMPLES=OFF \
    -DSDL2IMAGE_AVIF=OFF \
    -DSDL2IMAGE_JXL=OFF \
    -DSDL2IMAGE_TIF=OFF \
    -DSDL2IMAGE_WEBP=OFF

# SDL2_mixer: OGG Vorbis via the built-in stb_vorbis, MP3 via the built-in
# minimp3. Everything requiring an external codec library is disabled.
fetch "SDL2_mixer-$SDL2_MIXER_VERSION.tar.gz" \
    "https://github.com/libsdl-org/SDL_mixer/releases/download/release-$SDL2_MIXER_VERSION/SDL2_mixer-$SDL2_MIXER_VERSION.tar.gz"
build_cmake_dep "SDL2_mixer-$SDL2_MIXER_VERSION" \
    -DSDL2MIXER_VENDORED=ON \
    -DSDL2MIXER_SAMPLES=OFF \
    -DSDL2MIXER_CMD=OFF \
    -DSDL2MIXER_VORBIS=STB \
    -DSDL2MIXER_FLAC=OFF \
    -DSDL2MIXER_GME=OFF \
    -DSDL2MIXER_MOD=OFF \
    -DSDL2MIXER_OPUS=OFF \
    -DSDL2MIXER_WAVPACK=OFF

fetch "SDL2_net-$SDL2_NET_VERSION.tar.gz" \
    "https://github.com/libsdl-org/SDL_net/releases/download/release-$SDL2_NET_VERSION/SDL2_net-$SDL2_NET_VERSION.tar.gz"
build_cmake_dep "SDL2_net-$SDL2_NET_VERSION" \
    -DSDL2NET_SAMPLES=OFF

# SDL2_ttf: builds vendored freetype from the tarball's external/ dir
# (harfbuzz stays disabled to keep the build small)
fetch "SDL2_ttf-$SDL2_TTF_VERSION.tar.gz" \
    "https://github.com/libsdl-org/SDL_ttf/releases/download/release-$SDL2_TTF_VERSION/SDL2_ttf-$SDL2_TTF_VERSION.tar.gz"
build_cmake_dep "SDL2_ttf-$SDL2_TTF_VERSION" \
    -DSDL2TTF_VENDORED=ON \
    -DSDL2TTF_SAMPLES=OFF

fetch "physfs-$PHYSFS_VERSION.tar.gz" \
    "https://github.com/icculus/physfs/archive/refs/tags/release-$PHYSFS_VERSION.tar.gz"
mv "physfs-release-$PHYSFS_VERSION" "physfs-$PHYSFS_VERSION"
build_cmake_dep "physfs-$PHYSFS_VERSION" \
    -DPHYSFS_BUILD_STATIC=OFF \
    -DPHYSFS_BUILD_TEST=OFF \
    -DPHYSFS_BUILD_DOCS=OFF

# GNU libiconv: the macOS system iconv fails gnulib's "working iconv"
# test, which makes gettext's libtextstyle build fail at link time
fetch "libiconv-$LIBICONV_VERSION.tar.gz" \
    "https://ftp.gnu.org/pub/gnu/libiconv/libiconv-$LIBICONV_VERSION.tar.gz"
(
    cd "libiconv-$LIBICONV_VERSION"
    ./configure --prefix="$DEPS_PREFIX" --disable-static
    make -j"$JOBS"
    make install
)

# gettext: provides msgfmt/xgettext/msgmerge for the translations and
# libintl for the client. This previously came transitively via Homebrew.
fetch "gettext-$GETTEXT_VERSION.tar.xz" \
    "https://ftp.gnu.org/pub/gnu/gettext/gettext-$GETTEXT_VERSION.tar.xz"
(
    cd "gettext-$GETTEXT_VERSION"
    # The bundled gnulib/libtextstyle sources predate clang 16, which made
    # these diagnostics errors
    ./configure --prefix="$DEPS_PREFIX" \
        CFLAGS="-O2 -Wno-incompatible-function-pointer-types -Wno-deprecated-non-prototype" \
        --with-libiconv-prefix="$DEPS_PREFIX" \
        --disable-static \
        --disable-java \
        --disable-csharp \
        --without-emacs \
        --without-git \
        --without-cvs
    make -j"$JOBS"
    make install
)

# Verify the layout Mana's CMake expects
missing=0
for f in \
    bin/msgfmt bin/xgettext bin/msgmerge bin/pkg-config \
    lib/libSDL2.dylib lib/libSDL2_image.dylib lib/libSDL2_mixer.dylib \
    lib/libSDL2_net.dylib lib/libSDL2_ttf.dylib lib/libSDL3.dylib \
    lib/libphysfs.dylib lib/libintl.dylib lib/libiconv.dylib \
    lib/pkgconfig/sdl2-compat.pc include/SDL3/SDL.h; do
    if [ ! -e "$DEPS_PREFIX/$f" ]; then
        echo "MISSING: $DEPS_PREFIX/$f" >&2
        missing=1
    fi
done
[ "$missing" -eq 0 ] || exit 1

echo "$STAMP" > "$DEPS_PREFIX/.stamp"
echo "==> Dependencies installed to $DEPS_PREFIX"
