CMAKE_MINIMUM_REQUIRED(VERSION 3.12...3.27)

IF (NOT VERSION)
    SET(VERSION 0.6.1)
ELSE()
    STRING(REPLACE "." " " _VERSION ${VERSION})
    SEPARATE_ARGUMENTS(_VERSION)
    LIST(LENGTH _VERSION _LEN)
    IF(NOT _LEN EQUAL 4 AND NOT _LEN EQUAL 3)
        MESSAGE(FATAL_ERROR "Version needs to be in the form MAJOR.MINOR.RELEASE[.BUILD]")
    ENDIF()
ENDIF()

PROJECT(Mana
    VERSION ${VERSION}
    DESCRIPTION "Mana MMORPG Client"
    HOMEPAGE_URL "https://www.manasource.org")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# where to look for cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

FIND_PACKAGE(Gettext)

OPTION(WITH_OPENGL "Enable OpenGL support" ON)
OPTION(ENABLE_NLS "Enable building of translations" ON)
OPTION(ENABLE_MANASERV "Enable Manaserv support" ON)
OPTION(USE_SYSTEM_GUICHAN "Use system Guichan" ON)

IF (WIN32)
    SET(PKG_DATADIR ".")
    SET(LOCALEDIR "share/locale")
    SET(PKG_BINDIR ".")
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

SET(CPACK_PACKAGE_VENDOR "Mana Development Team")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}") # by default it includes the version
SET(CPACK_PACKAGE_EXECUTABLES "mana;Mana")
SET(CPACK_VERBATIM_VARIABLES TRUE)
IF(WIN32)
    SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/data/icons/mana.ico")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "mana.exe")
    SET(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_SOURCE_DIR}\\packaging\\windows\\setup_welcome.bmp")
    SET(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_HOMEPAGE_URL})
    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    SET(CPACK_NSIS_MUI_FINISHPAGE_RUN "mana.exe")
    SET(CPACK_NSIS_EXECUTABLES_DIRECTORY ${PKG_BINDIR})
    SET(CPACK_NSIS_MANIFEST_DPI_AWARE ON)

    install(FILES
      $ENV{MINGW_PREFIX}/bin/libgcc_s_seh-1.dll
      $ENV{MINGW_PREFIX}/bin/libwinpthread-1.dll
      $ENV{MINGW_PREFIX}/bin/libstdc++-6.dll
      $ENV{MINGW_PREFIX}/bin/libcurl-4.dll
      $ENV{MINGW_PREFIX}/bin/libintl-8.dll
      $ENV{MINGW_PREFIX}/bin/libphysfs.dll
      $ENV{MINGW_PREFIX}/bin/libpng16-16.dll
      $ENV{MINGW_PREFIX}/bin/SDL2.dll
      $ENV{MINGW_PREFIX}/bin/SDL2_image.dll
      $ENV{MINGW_PREFIX}/bin/SDL2_mixer.dll
      $ENV{MINGW_PREFIX}/bin/SDL2_net.dll
      $ENV{MINGW_PREFIX}/bin/SDL2_ttf.dll
      $ENV{MINGW_PREFIX}/bin/libxml2-2.dll
      $ENV{MINGW_PREFIX}/bin/zlib1.dll
      $ENV{MINGW_PREFIX}/bin/libiconv-2.dll
      $ENV{MINGW_PREFIX}/bin/libbrotlidec.dll
      $ENV{MINGW_PREFIX}/bin/libidn2-0.dll
      $ENV{MINGW_PREFIX}/bin/libpsl-5.dll
      $ENV{MINGW_PREFIX}/bin/libssh2-1.dll
      $ENV{MINGW_PREFIX}/bin/libzstd.dll
      $ENV{MINGW_PREFIX}/bin/libavif-16.dll
      $ENV{MINGW_PREFIX}/bin/libjxl.dll
      $ENV{MINGW_PREFIX}/bin/libjpeg-8.dll
      $ENV{MINGW_PREFIX}/bin/libtiff-6.dll
      $ENV{MINGW_PREFIX}/bin/libwebp-7.dll
      $ENV{MINGW_PREFIX}/bin/libwebpdemux-2.dll
      $ENV{MINGW_PREFIX}/bin/libmpg123-0.dll
      $ENV{MINGW_PREFIX}/bin/libopusfile-0.dll
      $ENV{MINGW_PREFIX}/bin/libfreetype-6.dll
      $ENV{MINGW_PREFIX}/bin/libharfbuzz-0.dll
      $ENV{MINGW_PREFIX}/bin/liblzma-5.dll
      $ENV{MINGW_PREFIX}/bin/libbrotlicommon.dll
      $ENV{MINGW_PREFIX}/bin/libunistring-5.dll
      $ENV{MINGW_PREFIX}/bin/libaom.dll
      $ENV{MINGW_PREFIX}/bin/libdav1d-7.dll
      $ENV{MINGW_PREFIX}/bin/libsharpyuv-0.dll
      $ENV{MINGW_PREFIX}/bin/libSvtAv1Enc-2.dll
      $ENV{MINGW_PREFIX}/bin/libyuv.dll
      $ENV{MINGW_PREFIX}/bin/libbrotlienc.dll
      $ENV{MINGW_PREFIX}/bin/libhwy.dll
      $ENV{MINGW_PREFIX}/bin/libjxl_cms.dll
      $ENV{MINGW_PREFIX}/bin/libdeflate.dll
      $ENV{MINGW_PREFIX}/bin/libjbig-0.dll
      $ENV{MINGW_PREFIX}/bin/libLerc.dll
      $ENV{MINGW_PREFIX}/bin/libopus-0.dll
      $ENV{MINGW_PREFIX}/bin/rav1e.dll
      $ENV{MINGW_PREFIX}/bin/libbz2-1.dll
      $ENV{MINGW_PREFIX}/bin/libglib-2.0-0.dll
      $ENV{MINGW_PREFIX}/bin/libogg-0.dll
      $ENV{MINGW_PREFIX}/bin/libgraphite2.dll
      $ENV{MINGW_PREFIX}/bin/liblcms2-2.dll
      $ENV{MINGW_PREFIX}/bin/libpcre2-8-0.dll
      DESTINATION ${PKG_BINDIR}
    )
ENDIF()
INCLUDE(CPack)
