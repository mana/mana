find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET
  sdl2>=2.0.18
  SDL2_image)

add_executable(dyecmd
  src/dyecmd.cpp
  ${CMAKE_SOURCE_DIR}/src/log.cpp
  ${CMAKE_SOURCE_DIR}/src/resources/dye.cpp)

target_include_directories(dyecmd PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(dyecmd PRIVATE PkgConfig::SDL2)
