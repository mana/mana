# - Try to find SDL_gfx
# Once done this will define
#
#  SDLGFX_FOUND - system has SDL_gfx
#  SDLGFX_INCLUDE_DIR - the SDL_gfx include directory

# Be quiet if SDL_gfx was already found
IF (SDL_gfx_INCLUDE_DIR AND SDL_gfx_LIBRARY)
    SET(SDL_gfx_QUIET TRUE)
ENDIF (SDL_gfx_INCLUDE_DIR AND SDL_gfx_LIBRARY)


FIND_PATH(SDL_gfx_INCLUDE_DIR
    NAMES SDL_rotozoom.h
    PATH_SUFFIXES SDL
    )

FIND_LIBRARY(SDL_gfx_LIBRARY
    NAMES SDL_gfx
    PATHS /usr/lib /usr/local/lib
    )

MARK_AS_ADVANCED(SDL_gfx_INCLUDE_DIR SDL_gfx_LIBRARY)


IF (SDL_gfx_INCLUDE_DIR AND SDL_gfx_LIBRARY)
    SET(SDLGFX_FOUND TRUE)
    SET(SDLGFX_INCLUDE_DIR ${SDL_gfx_INCLUDE_DIR})
    SET(SDLGFX_LIBRARIES ${SDL_gfx_LIBRARY})

    IF (NOT SDL_gfx_FIND_QUIETLY AND NOT SDL_gfx_QUIET)
        MESSAGE(STATUS "Found SDL_gfx: ${SDL_gfx_LIBRARY}")
    ENDIF (NOT SDL_gfx_FIND_QUIETLY AND NOT SDL_gfx_QUIET)
ENDIF (SDL_gfx_INCLUDE_DIR AND SDL_gfx_LIBRARY)

# Bail out if SDL_gfx is not found but required
IF (NOT SDLGFX_FOUND AND SDL_gfx_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find SDL_gfx library ${SDLGFX_INCLUDE_DIR}")
ENDIF (NOT SDLGFX_FOUND AND SDL_gfx_FIND_REQUIRED)
