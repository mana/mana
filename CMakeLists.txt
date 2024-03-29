CMAKE_MINIMUM_REQUIRED(VERSION 3.8...3.27)

PROJECT(MANA)

IF (NOT VERSION)
    SET(VERSION 0.6.1)
ENDIF()

set (CMAKE_CXX_STANDARD 17)

STRING(REPLACE "." " " _VERSION ${VERSION})
SEPARATE_ARGUMENTS(_VERSION)
LIST(LENGTH _VERSION _LEN)
IF(NOT _LEN EQUAL 4 AND NOT _LEN EQUAL 3)
    MESSAGE(FATAL_ERROR "Version needs to be in the form MAJOR.MINOR.RELEASE[.BUILD]")
ENDIF()

LIST(GET _VERSION 0 VER_MAJOR)
LIST(GET _VERSION 1 VER_MINOR)
LIST(GET _VERSION 2 VER_RELEASE)
IF(_LEN EQUAL 4)
    LIST(GET _VERSION 3 VER_BUILD)
ELSE()
    SET(VER_BUILD 0)
ENDIF()

# where to look for cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

FIND_PACKAGE(Gettext)

OPTION(WITH_OPENGL "Enable OpenGL support" ON)
OPTION(ENABLE_NLS "Enable building of tranlations" ON)
OPTION(ENABLE_MANASERV "Enable Manaserv support" ON)
OPTION(USE_SYSTEM_GUICHAN "Use system Guichan" ON)

IF (WIN32)
    SET(PKG_DATADIR ".")
    SET(PKG_BINDIR ".")
    SET(LOCALEDIR ".")
    CONFIGURE_FILE(src/winver.h.in src/winver.h)
ELSE (WIN32)
    SET(PKG_DATADIR ${CMAKE_INSTALL_PREFIX}/share/mana CACHE PATH "Mana datadir")
    SET(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
    SET(PKG_BINDIR ${CMAKE_INSTALL_PREFIX}/bin CACHE PATH "Mana bindir")
ENDIF (WIN32)

ADD_SUBDIRECTORY(data)

IF (ENABLE_MANASERV)
    ADD_SUBDIRECTORY(libs/enet)
ENDIF()

# On macOS we need to build Guichan ourselves since it is disabled in Homebrew
# This might also be a good idea for the Windows build
if(APPLE OR NOT USE_SYSTEM_GUICHAN)
    # This policy makes sure that the below options are not ignored by the
    # added Guichan CMakeLists.txt
    SET(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    SET(ENABLE_ALLEGRO OFF)
    SET(ENABLE_IRRLICHT OFF)
    SET(ENABLE_SDL OFF)
    SET(ENABLE_OPENGL OFF)
    SET(BUILD_GUICHAN_SHARED OFF)
    ADD_SUBDIRECTORY(libs/guichan)
endif()

ADD_SUBDIRECTORY(src)

IF (GETTEXT_FOUND AND ENABLE_NLS)
    ADD_SUBDIRECTORY(po)
ENDIF()

If(UNIX)
    INSTALL(FILES org.manasource.Mana.desktop DESTINATION share/applications)
    INSTALL(FILES org.manasource.Mana.metainfo.xml DESTINATION share/metainfo)
    INSTALL(FILES data/icons/mana.png DESTINATION share/icons/hicolor/128x128/apps RENAME org.manasource.Mana.png)
    INSTALL(FILES data/icons/mana.svg DESTINATION share/icons/hicolor/scalable/apps RENAME org.manasource.Mana.svg)
ENDIF()

SET(CPACK_PACKAGE_NAME "mana")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mana")
SET(CPACK_PACKAGE_VENDOR "Mana Development Team")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Mana")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VER_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VER_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VER_RELEASE})
IF(WIN32)
    SET(CPACK_NSIS_DISPLAY_NAME "Mana")
    SET(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/data/icons/mana.ico")
    SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/packaging/windows/setup_welcome.bmp")
    SET(CPACK_NSIS_URL_INFO_ABOUT "https://www.manasource.org")
ENDIF()
INCLUDE(CPack)
