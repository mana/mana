0. Contents
------------
 1. Basic syntax
 2. How do I...
 3. Crosscompiling using CMake
 4. Creating an installer binary for Windows
 5. Building on OS X

This readme explains the most common parameters to CMake needed for
building mana, as well as setting up a cross build environement to
create Windows builds.


1. Basic syntax
---------------

cmake [options] <source directory>

If you don't need any special options just change to the directory where
you extracted the sources and do `cmake . && make'

The syntax for setting variables to control CMakes behaviour is
-D <variable>=<value>


2. How do I...
--------------

- Use a custom install prefix (like --prefix on autoconf)?
  CMAKE_INSTALL_PREFIX=/path/to/prefix
- Create a debug build?
  CMAKE_BUILD_TYPE=Debug .
- Add additional package search directories?
  CMAKE_PREFIX_PATH=/prefix/path
- Add additional include search directories?
  CMAKE_INCLUDE_PATH=/include/path

For example, to build mana to install in /opt/mana, with libraries in
/build/mana/lib, and SDL-headers in /build/mana/include/SDL you'd use
the following command:

cmake -D CMAKE_PREFIX_PATH=/build/mana \
  -D CMAKE_INCLUDE_PATH=/build/mana/include/SDL \
  -D CMAKE_INSTALL_PREFIX=/opt/mana .


3. Crosscompiling using CMake
-----------------------------

The following example assumes you're doing a Windows-build from within a
UNIX environement, using mingw32 installed in /build/mingw32.

- create a toolchain-file describing your environement:
$ cat /build/toolchain.cmake
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# toolchain prefix, can be overridden by -DTOOLCHAIN=...
IF (NOT TOOLCHAIN)
    SET(TOOLCHAIN "i386-mingw32-")
ENDIF()

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${TOOLCHAIN}gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN}g++)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /build/mingw32 /build/mana-libs )

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


4. Creating an installer binary for Windows
-------------------------------------------

Call cmake with the following flags:

 - CMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake
   The path to your toolchain file from chapter 3
 - VERSION=$VERSION
   The version number for the build, used to set client identification and
   EXE meta information. Needs to be in the form Major.Minor.Release.Build
 - CMAKE_BUILD_TYPE=RelWithDebInfo|Release
   RelWithDebInfo is the prefered build type, as it provides some backtrace
   information without blowing the binary up too much. Don't use Debug.

Install nsis (on Debian, apt-get install nsis), and get the UnTGZ plugin:
    http://nsis.sourceforge.net/UnTGZ_plug-in
Place untgz.dll in nsis plugins directory (on Debian, /usr/share/nsis/Plugins/)

Now chdir to packaging/windows. Make sure you have the msgfmt program (from
gettext), and execute make-translations.sh to generate translations from the
po-files in ../../translations/

Check that you've got the translations, and call `makensis setup.nsi' with the
following parameters:

 -DDLLDIR=/build/mana-libs/lib/
  The directory you've put the libraries. It's the one from the toolchain file
  with /lib/ added at the end. If you'd like to include the Dr. MingW crash
  handler place exchndl.dll in this directory.
 -DPRODUCT_VERSION=$VERSION
  Same as above
 -DUPX=true
  Set to upx if you'd like to compress the binaries using upx (and install upx,
  of course). Will remove debug symbols.
 -DEXESUFFIX=/src
  Suffix to the source directory, pointing to the directory where the binaries
  are. For cmake-builds that's /src.

To build the installer for the 0.1.0.0 release you'd do the following:
$ cmake -DVERSION=0.1.0.0 -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake .
[...]
$ make
[...]
$ cd packaging/windows
$ ./make-translations.sh
[...]
$ makensis -DDLLDIR=/build/mana-libs/lib/ -DPRODUCT_VERSION=0.1.0.0 \
           -DUPX=true -DEXESUFFIX=/src setup.nsi

and end up with the installer in mana-0.1.0.0-win32.exe

5. Building on OS X
-------------------

In your mana directory:

$ export CC=/usr/bin/clang
$ export CXX=/usr/bin/clang++
$ cmake -DENABLE_CPP0X=OFF
$ make
$ src/mana
