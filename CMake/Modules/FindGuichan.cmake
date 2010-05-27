# - Try to find Guichan
# Once done this will define
#
#  GUICHAN_FOUND - system has Guichan
#  GUICHAN_INCLUDE_DIR - the Guichan include directory
#  GUICHAN_LIBRARIES - the libraries needed to use Guichan
#
# Supported components: SDL OpenGL
#
# TODO: Clean up the components stuff and try to add all known
# components when none are specified. May not bail out for not found
# components then, set some var instead...

# Be quiet if Guichan was already found
# We need an extra variable to have the check per component

IF (Guichan_INCLUDE_DIR AND Guichan_LIBRARY)
    SET(Guichan_QUIET TRUE)
ENDIF ()

FIND_PATH(Guichan_INCLUDE_DIR
    NAMES guichan.hpp
    PATHS /usr/include /usr/local/include
    )

FIND_LIBRARY(Guichan_LIBRARY
    NAMES guichan
    PATHS /usr/lib /usr/local/lib
    )

MARK_AS_ADVANCED(Guichan_INCLUDE_DIR Guichan_LIBRARY)

IF (Guichan_INCLUDE_DIR AND Guichan_LIBRARY)
    SET(GUICHAN_FOUND TRUE)
    SET(GUICHAN_INCLUDE_DIR ${Guichan_INCLUDE_DIR})
    SET(GUICHAN_LIBRARIES ${Guichan_LIBRARY})

    IF (NOT Guichan_FIND_QUIETLY AND NOT Guichan_QUIET)
        MESSAGE(STATUS "Found Guichan: ${Guichan_LIBRARY}")
    ENDIF ()
ENDIF ()

# Bail out if Guichan is not found but required
IF (NOT GUICHAN_FOUND AND Guichan_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could NOT find Guichan library ${GUICHAN_INCLUDE_DIR}")
ENDIF ()

# Check for guichan components
FOREACH (COMPONENT ${Guichan_FIND_COMPONENTS})
    SET(COMPONENT_FOUND FALSE)

    IF (COMPONENT STREQUAL "SDL")
        SET(COMPONENT_FOUND TRUE)
        SET(Guichan_HEADER "guichan/sdl.hpp")
        SET(Guichan_LIBRARY "guichan_sdl")
    ENDIF ()

    IF (COMPONENT STREQUAL "OpenGL")
        SET(COMPONENT_FOUND TRUE)
        SET(Guichan_HEADER "guichan/opengl.hpp")
        SET(Guichan_LIBRARY "guichan_opengl")
    ENDIF ()

    IF (NOT COMPONENT_FOUND)
        MESSAGE(FATAL_ERROR "Invalid Guichan component: ${COMPONENT}")
    ENDIF (NOT COMPONENT_FOUND)

    # Check if we already found this component
    IF (Guichan_LIBRARY_${COMPONENT})
        SET(Guichan_QUIET TRUE)
    ELSE ()
        SET(Guichan_QUIET FALSE)
    ENDIF ()

    FIND_LIBRARY(Guichan_LIBRARY_${COMPONENT}
        NAMES ${Guichan_LIBRARY}
        PATHS /usr/lib /usr/local/lib
        )

    MARK_AS_ADVANCED(Guichan_LIBRARY_${COMPONENT})

    IF (NOT EXISTS ${Guichan_INCLUDE_DIR}/${Guichan_HEADER} OR
            NOT Guichan_LIBRARY_${COMPONENT})
        SET(GUICHAN_FOUND FALSE)
        MESSAGE(FATAL_ERROR "Could NOT find Guichan component ${COMPONENT}")
    ENDIF ()

    # Check guichan >= 0.8.0 by searching for the widgetlistener.hpp file.
    SET(Guichan_HEADER "guichan/widgetlistener.hpp")
    IF (NOT EXISTS ${Guichan_INCLUDE_DIR}/${Guichan_HEADER})
            SET(GUICHAN_FOUND FALSE)
            MESSAGE(FATAL_ERROR "Guichan version is less than 0.8.0")
    ENDIF()

    IF (NOT Guichan_FIND_QUIETLY AND NOT Guichan_QUIET)
        MESSAGE(STATUS "Found Guichan ${COMPONENT}: ${Guichan_LIBRARY_${COMPONENT}}")
    ENDIF ()

    SET(GUICHAN_LIBRARIES ${GUICHAN_LIBRARIES} ${Guichan_LIBRARY_${COMPONENT}})
ENDFOREACH()
