cmake_minimum_required(VERSION 2.6)

project(MANA_DYECOMMAND)

if(NOT VERSION)
  set(VERSION 1.0.0)
endif()

string(REPLACE "." " " _VERSION ${VERSION})
separate_arguments(_VERSION)
list(LENGTH _VERSION _LEN)
if(NOT (_LEN EQUAL 4 OR _LEN EQUAL 3))
  message(
    FATAL_ERROR "Version needs to be in the form MAJOR.MINOR.RELEASE[.BUILD]")
endif()

list(GET _VERSION 0 VER_MAJOR)
list(GET _VERSION 1 VER_MINOR)
list(GET _VERSION 2 VER_RELEASE)
if(_LEN EQUAL 4)
  list(GET _VERSION 3 VER_BUILD)
else()
  set(VER_BUILD 0)
endif()

# where to look for cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

add_subdirectory(src)
