FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_image REQUIRED)
FIND_PACKAGE(PNG REQUIRED)

IF (CMAKE_COMPILER_IS_GNUCXX)
    # Help getting compilation warnings
    SET(CMAKE_CXX_FLAGS "-Wall")
    IF (WIN32)
        # This includes enough debug information to get something useful
        # from Dr. Mingw while keeping binary size down. Almost useless
        # with gdb, though.
        SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb0 -gstabs2")
    ENDIF()
ENDIF()

SET(FLAGS "-DPACKAGE_VERSION=\\\"${VERSION}\\\"")
SET(FLAGS "${FLAGS} -DPKG_DATADIR=\\\"${PKG_DATADIR}/\\\"")
SET(FLAGS "${FLAGS} -DLOCALEDIR=\\\"${LOCALEDIR}/\\\"")

IF (CMAKE_BUILD_TYPE)
    STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
    IF((CMAKE_BUILD_TYPE_TOLOWER MATCHES debug) OR
       (CMAKE_BUILD_TYPE_TOLOWER MATCHES relwithdebinfo))
        SET(FLAGS "${FLAGS} -DDEBUG")
    ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL_INCLUDE_DIR}
    ${SDLIMAGE_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    )

# Fix some stuff that gets not hidden by mainline modules
MARK_AS_ADVANCED(SDLIMAGE_INCLUDE_DIR)
MARK_AS_ADVANCED(SDLIMAGE_LIBRARY)
MARK_AS_ADVANCED(SDLMAIN_LIBRARY)
MARK_AS_ADVANCED(SDL_INCLUDE_DIR)
MARK_AS_ADVANCED(SDL_LIBRARY)

SET(SRCS
    dye.cpp
    dye.h
    dyecmd.cpp
    imagewriter.cpp
    imagewriter.h
    )

SET (PROGRAMS dyecmd)

ADD_EXECUTABLE(dyecmd WIN32 ${SRCS})

TARGET_LINK_LIBRARIES(dyecmd
    ${SDL_LIBRARY}
    ${SDLIMAGE_LIBRARY}
    ${PNG_LIBRARIES})

SET_TARGET_PROPERTIES(dyecmd PROPERTIES COMPILE_FLAGS "${FLAGS}")
