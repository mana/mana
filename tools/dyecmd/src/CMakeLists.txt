find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(PNG REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
  # Help getting compilation warnings
  set(CMAKE_CXX_FLAGS "-Wall")
  if(WIN32)
    # This includes enough debug information to get something useful from Dr.
    # Mingw while keeping binary size down. Almost useless with gdb, though.
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb0 -gstabs2")
  endif()
endif()

set(FLAGS "-DPACKAGE_VERSION=\\\"${VERSION}\\\"")
set(FLAGS "${FLAGS} -DPKG_DATADIR=\\\"${PKG_DATADIR}/\\\"")
set(FLAGS "${FLAGS} -DLOCALEDIR=\\\"${LOCALEDIR}/\\\"")

if(CMAKE_BUILD_TYPE)
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
  if((CMAKE_BUILD_TYPE_TOLOWER MATCHES debug) OR (CMAKE_BUILD_TYPE_TOLOWER
                                                  MATCHES relwithdebinfo))
    set(FLAGS "${FLAGS} -DDEBUG")
  endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SDL_INCLUDE_DIR}
                    ${SDLIMAGE_INCLUDE_DIR} ${PNG_INCLUDE_DIR})

# Fix some stuff that gets not hidden by mainline modules
mark_as_advanced(SDLIMAGE_INCLUDE_DIR)
mark_as_advanced(SDLIMAGE_LIBRARY)
mark_as_advanced(SDLMAIN_LIBRARY)
mark_as_advanced(SDL_INCLUDE_DIR)
mark_as_advanced(SDL_LIBRARY)

set(SRCS dye.cpp dye.h dyecmd.cpp imagewriter.cpp imagewriter.h)

add_executable(dyecmd WIN32 ${SRCS})

target_link_libraries(dyecmd ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY}
                      ${PNG_LIBRARIES})

set_target_properties(dyecmd PROPERTIES COMPILE_FLAGS "${FLAGS}")
