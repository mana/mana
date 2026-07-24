cmake_minimum_required(VERSION 3.22...4.2)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

if(NOT VERSION)
  set(VERSION 0.8.0)
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

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${PROJECT_NAME})

option(WITH_OPENGL "Enable OpenGL support" ON)
option(ENABLE_NLS "Enable building of translations" ON)
option(ENABLE_MANASERV "Enable Manaserv support" ON)
option(USE_SYSTEM_ENET "Use system ENet" ON)
option(USE_SYSTEM_GUICHAN "Use system Guichan" ON)

# Translations are on by default and hard-require the gettext tools, so that a
# machine without them fails to configure instead of silently producing a build
# with no catalogs. Turn them off explicitly with -DENABLE_NLS=OFF.
if(ENABLE_NLS)
  find_package(Gettext REQUIRED)
endif()

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
  # The ENet CMakeLists.txt declares compatibility with CMake versions older
  # than 3.5, which CMake 4 refuses to configure without this variable
  set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
  add_subdirectory(libs/enet EXCLUDE_FROM_ALL)
  unset(CMAKE_POLICY_VERSION_MINIMUM)
endif()

if(NOT USE_SYSTEM_GUICHAN)
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

if(ENABLE_NLS)
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
  set(_portable_marker_path "portable.xml")
  if(NOT CMAKE_INSTALL_BINDIR STREQUAL "." AND NOT CMAKE_INSTALL_BINDIR STREQUAL "")
    set(_portable_marker_path "${CMAKE_INSTALL_BINDIR}/portable.xml")
  endif()
  string(REPLACE "/" "\\\\" PORTABLE_MARKER_PATH_NSIS "${_portable_marker_path}")
  configure_file(
    "${CMAKE_SOURCE_DIR}/packaging/windows/nsis-portable-guard.nsi.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nsis-portable-guard.nsi"
    @ONLY
  )
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/nsis-portable-guard.nsi" _nsis_portable_check)
  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${_nsis_portable_check}")

  # Name the installer after the target architecture (defaults to "win64")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|aarch64)$"
     OR "$ENV{MSYSTEM_CARCH}" STREQUAL "aarch64")
    set(CPACK_SYSTEM_NAME "win-arm64")
  endif()

  # Install portable.xml as part of an optional Portable component
  set(CPACK_COMPONENT_PORTABLE_DISPLAY_NAME "Portable")
  set(CPACK_COMPONENT_PORTABLE_DESCRIPTION "Portable mode. Settings and updates are stored in the installation folder.")
  set(CPACK_COMPONENT_PORTABLE_DISABLED TRUE)
  install(
    FILES "${CMAKE_SOURCE_DIR}/packaging/windows/portable.xml"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT Portable
  )
endif()

set(CPACK_COMPONENT_MANA_DISPLAY_NAME "${PROJECT_NAME}")
set(CPACK_COMPONENT_MANA_DESCRIPTION "${PROJECT_DESCRIPTION}.")
set(CPACK_COMPONENT_MANA_REQUIRED TRUE)

set(CPACK_COMPONENT_TRANSLATIONS_DISPLAY_NAME "Translations")
set(CPACK_COMPONENT_TRANSLATIONS_DESCRIPTION "Translations for the user interface. Without these only English will be available.")

include(CPack)
