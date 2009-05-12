This readme explains the most common parameters to CMake needed for 
building tmw.

Basic syntax
------------

cmake [options] <source directory>

If you don't need any special options just change to the directory where 
you extracted the sources and do `cmake . && make'

The syntax for setting variables to control CMakes behaviour is 
-D <variable>=<value>


How do I...
-----------

- Use a custom install prefix (like --prefix on autoconf)?
  CMAKE_INSTALL_PREFIX=/path/to/prefix
- Create a debug build?
  CMAKE_BUILD_TYPE=Debug .
- Add additional package search directories?
  CMAKE_PREFIX_PATH=/prefix/path
- Add additional include search directories?
  CMAKE_INCLUDE_PATH=/include/path

For example, to build tmw to install in /opt/tmw, with libraries in 
/build/tmw/lib, and SDL-headers in /build/tmw/include/SDL you'd use 
the following command:

cmake -D CMAKE_PREFIX_PATH=/build/tmw \
  -D CMAKE_INCLUDE_PATH=/build/tmw/include/SDL \
  -D CMAKE_INSTALL_PREFIX=/opt/tmw .


Crosscompiling using CMake
--------------------------

The following example assumes you're doing a Windows-build from within a
UNIX environement, using mingw32 installed in /build/mingw32.

- create a toolchain-file describing your environement:
$ cat /build/toolchain.cmake
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i386-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i386-mingw32-g++)
# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /build/mingw32 /build/tmw-libs )
# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

- set your PATH to include the bin-directory of your mingw32-installation:
$ export PATH=/build/mingw32/bin:$PATH

- configure the source tree for the build, using the toolchain file:
$ cmake -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake . 

- use make for building the application

