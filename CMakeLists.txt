cmake_minimum_required(VERSION 3.12...3.27)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

if(NOT VERSION)
  set(VERSION 0.6.1)
else()
  string(REPLACE "." " " _VERSION ${VERSION})
  separate_arguments(_VERSION)
  list(LENGTH _VERSION _LEN)
  if(NOT _LEN EQUAL 4 AND NOT _LEN EQUAL 3)
    message(
      FATAL_ERROR "Version needs to be in the form MAJOR.MINOR.RELEASE[.BUILD]")
  endif()
endif()

project(
  Mana
  VERSION ${VERSION}
  DESCRIPTION "Mana MMORPG Client"
  HOMEPAGE_URL "https://www.manasource.org")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# where to look for cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

find_package(Gettext)

option(WITH_OPENGL "Enable OpenGL support" ON)
option(ENABLE_NLS "Enable building of translations" ON)
option(ENABLE_MANASERV "Enable Manaserv support" ON)
option(USE_SYSTEM_ENET "Use system ENet" OFF)
option(USE_SYSTEM_GUICHAN "Use system Guichan" ON)

if(WIN32)
  set(CMAKE_INSTALL_DATADIR ".")
  set(CMAKE_INSTALL_LOCALEDIR "translations")
  set(CMAKE_INSTALL_BINDIR ".")
elseif(APPLE)
  set(CMAKE_INSTALL_DATAROOTDIR "Mana.app/Contents")
  set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_DATAROOTDIR}/Resources")
  set(CMAKE_INSTALL_LOCALEDIR "${CMAKE_INSTALL_DATADIR}/Translations")
else()
  set(CMAKE_INSTALL_DATADIR "share/mana")
endif()

include(GNUInstallDirs)

add_subdirectory(data)

if(ENABLE_MANASERV AND NOT USE_SYSTEM_ENET)
  add_subdirectory(libs/enet EXCLUDE_FROM_ALL)
endif()

# On macOS we need to build Guichan ourselves since it is disabled in Homebrew
# This might also be a good idea for the Windows build
if(APPLE OR NOT USE_SYSTEM_GUICHAN)
  # This policy makes sure that the below options are not ignored by the added
  # Guichan CMakeLists.txt
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(ENABLE_ALLEGRO OFF)
  set(ENABLE_IRRLICHT OFF)
  set(ENABLE_SDL OFF)
  set(ENABLE_OPENGL OFF)
  set(BUILD_GUICHAN_SHARED OFF)
  add_subdirectory(libs/guichan EXCLUDE_FROM_ALL)
endif()

# Set generic CPack properties here so they may be used in subdirectories
set(CPACK_PACKAGE_VENDOR "Mana Development Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
# By default the install directory includes the version
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_EXECUTABLES "mana;Mana")
set(CPACK_VERBATIM_VARIABLES TRUE)

add_subdirectory(src)

if(GETTEXT_FOUND AND ENABLE_NLS)
  add_subdirectory(po)
endif()

if(UNIX AND NOT APPLE)
  install(FILES org.manasource.Mana.desktop
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
  install(FILES org.manasource.Mana.metainfo.xml
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
  install(
    FILES data/icons/mana.png
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps
    RENAME org.manasource.Mana.png)
  install(
    FILES data/icons/mana.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
    RENAME org.manasource.Mana.svg)
endif()

# Windows NSIS installer
if(WIN32)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/data/icons/mana.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "mana.exe")
  set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP
      "${CMAKE_SOURCE_DIR}\\packaging\\windows\\setup_welcome.bmp")
  set(CPACK_NSIS_URL_INFO_ABOUT ${PROJECT_HOMEPAGE_URL})
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "mana.exe")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR})
  set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)

  install(
    FILES $ENV{MINGW_PREFIX}/bin/libgcc_s_seh-1.dll
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
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
include(CPack)
