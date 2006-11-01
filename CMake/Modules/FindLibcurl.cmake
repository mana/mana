# - Try to find libcurl
# Once done this will define
#
#  LIBCURL_FOUND - system has libcurl
#  LIBCURL_INCLUDE_DIR - the libcurl include directory
#  LIBCURL_LIBRARIES - the libraries needed to use libcurl
#  LIBCURL_DEFINITIONS - Compiler switches required for using libcurl

IF (Libcurl_INCLUDE_DIR AND Libcurl_LIBRARY)
   SET(Libcurl_FIND_QUIETLY TRUE)
ENDIF (Libcurl_INCLUDE_DIR AND Libcurl_LIBRARY)

FIND_PATH(Libcurl_INCLUDE_DIR curl/curl.h
    /usr/include
    /usr/local/include
    )

FIND_LIBRARY(Libcurl_LIBRARY
    NAMES curl
    PATHS /usr/lib /usr/local/lib
    )

IF (Libcurl_INCLUDE_DIR AND Libcurl_LIBRARY)
    SET(LIBCURL_FOUND TRUE)
    SET(LIBCURL_INCLUDE_DIR ${Libcurl_INCLUDE_DIR})
    SET(LIBCURL_LIBRARIES ${Libcurl_LIBRARY})
ELSE (Libcurl_INCLUDE_DIR AND Libcurl_LIBRARY)
    SET(LIBCURL_FOUND TRUE)
ENDIF (Libcurl_INCLUDE_DIR AND Libcurl_LIBRARY)

IF (LIBCURL_FOUND)
    IF (NOT Libcurl_FIND_QUIETLY)
        MESSAGE(STATUS "Found libcurl: ${Libcurl_LIBRARY}")
    ENDIF (NOT Libcurl_FIND_QUIETLY)
ELSE (LIBCURL_FOUND)
    IF (Libcurl_FIND_REQUIRED)
        MESSAGE(SEND_ERROR "Could NOT find libcurl")
    ENDIF (Libcurl_FIND_REQUIRED)
ENDIF (LIBCURL_FOUND)

MARK_AS_ADVANCED(Libcurl_INCLUDE_DIR Libcurl_LIBRARY)
