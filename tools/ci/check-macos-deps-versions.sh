#!/usr/bin/env bash
#
# Compare the dependency versions pinned in build-macos-deps.sh with the
# latest upstream releases. Manual check only; nothing is written.
#
# Only releases in the pinned major series are compared: the SDL2
# satellite libraries intentionally track their 2.x lines (the SDL2 API,
# provided by sdl2-compat), and a major bump anywhere needs a conscious
# look at the changelog anyway.
#
# Implemented with Devin (SWE-2).

set -uo pipefail

DEPS_SCRIPT="$(dirname "$0")/build-macos-deps.sh"

pin() { # <VAR name without _VERSION>
    sed -n "s/^$1_VERSION=//p" "$DEPS_SCRIPT"
}

# Version-sort dotted numbers (BSD sort lacks -V)
vsort() { sort -u -t. -k1,1n -k2,2n -k3,3n -k4,4n; }

# Filter to releases in the same major series as the pinned version
series() { grep -E "^${1%%.*}\\."; }

latest_gh() { # <owner/repo> <pinned>
    curl -fs --retry 2 \
        "https://api.github.com/repos/$1/releases?per_page=50" 2>/dev/null |
        sed -n 's/.*"tag_name": *"release-\([^"]*\)".*/\1/p' |
        series "$2" | vsort | tail -1
}

latest_dir() { # <index url> <file prefix> <pinned>
    curl -fs --retry 2 "$1" 2>/dev/null |
        sed -n "s/.*$2-\([0-9][0-9.]*\)\.tar\..*/\1/p" |
        series "$3" | vsort | tail -1
}

report() { # <name> <pinned> <latest>
    local status="ok"
    [ -z "$3" ] && status="no data"
    [ -n "$3" ] && [ "$2" != "$3" ] && status="UPDATE"
    printf '%-12s %-10s %-10s %s\n' "$1" "$2" "${3:-?}" "$status"
    [ "$status" = "UPDATE" ]
}

P_SDL3=$(pin SDL3)
P_SDL2_COMPAT=$(pin SDL2_COMPAT)
P_SDL2_IMAGE=$(pin SDL2_IMAGE)
P_SDL2_MIXER=$(pin SDL2_MIXER)
P_SDL2_NET=$(pin SDL2_NET)
P_SDL2_TTF=$(pin SDL2_TTF)
P_PHYSFS=$(pin PHYSFS)
P_LIBICONV=$(pin LIBICONV)
P_GETTEXT=$(pin GETTEXT)
P_PKGCONF=$(pin PKGCONF)

printf '%-12s %-10s %-10s %s\n' DEP PINNED LATEST STATUS
updates=0

report "SDL3"        "$P_SDL3"        "$(latest_gh libsdl-org/SDL "$P_SDL3")"                && updates=$((updates+1))
report "sdl2-compat" "$P_SDL2_COMPAT" "$(latest_gh libsdl-org/sdl2-compat "$P_SDL2_COMPAT")" && updates=$((updates+1))
report "SDL2_image"  "$P_SDL2_IMAGE"  "$(latest_gh libsdl-org/SDL_image "$P_SDL2_IMAGE")"    && updates=$((updates+1))
report "SDL2_mixer"  "$P_SDL2_MIXER"  "$(latest_gh libsdl-org/SDL_mixer "$P_SDL2_MIXER")"    && updates=$((updates+1))
report "SDL2_net"    "$P_SDL2_NET"    "$(latest_gh libsdl-org/SDL_net "$P_SDL2_NET")"        && updates=$((updates+1))
report "SDL2_ttf"    "$P_SDL2_TTF"    "$(latest_gh libsdl-org/SDL_ttf "$P_SDL2_TTF")"        && updates=$((updates+1))
report "physfs"      "$P_PHYSFS"      "$(latest_gh icculus/physfs "$P_PHYSFS")"              && updates=$((updates+1))
report "libiconv"    "$P_LIBICONV"    "$(latest_dir https://ftp.gnu.org/pub/gnu/libiconv/ libiconv "$P_LIBICONV")" && updates=$((updates+1))
report "gettext"     "$P_GETTEXT"     "$(latest_dir https://ftp.gnu.org/pub/gnu/gettext/ gettext "$P_GETTEXT")"    && updates=$((updates+1))
report "pkgconf"     "$P_PKGCONF"     "$(latest_dir https://distfiles.ariadne.space/pkgconf/ pkgconf "$P_PKGCONF")" && updates=$((updates+1))

[ "$updates" -eq 0 ] && echo "All pins current." || echo "$updates update(s) available."
