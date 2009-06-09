AC_PREREQ(2.59)
AC_INIT([The Mana World], [0.0.29.1], [themanaworld-devel@lists.sourceforge.net], [tmw])
AM_INIT_AUTOMAKE([1.9])
AC_CONFIG_HEADERS([config.h:config.h.in])
AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit floor getcwd gethostbyname memset mkdir select socket])

# Checks for internationalization support
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.16.1])

# Search for *-config
AC_PATH_PROG(SDL_CONFIG, sdl-config)
AC_PATH_PROG(PKG_CONFIG, pkg-config)
AC_PATH_PROG(CURL_CONFIG, curl-config)

# Checks for libraries
AC_CHECK_LIB([pthread], [pthread_create], ,
AC_MSG_ERROR([ *** Unable to find pthread library]))

AC_CHECK_LIB([guichan], [gcnGuichanVersion], ,
AC_MSG_ERROR([ *** Unable to find Guichan library (http://guichan.sf.net/)]))
AC_CHECK_HEADERS([guichan.hpp], ,
AC_MSG_ERROR([ *** Guichan library found but cannot find headers (http://guichan.sf.net/)]))

if test -n "$SDL_CONFIG"; then
    LIBS="$LIBS `$SDL_CONFIG --libs`"
    CPPFLAGS="$CPPFLAGS `$SDL_CONFIG --cflags`"
fi
AC_CHECK_LIB([SDL], [SDL_Init], ,
AC_MSG_ERROR([ *** Unable to find SDL library (http://www.libsdl.org/)]))
AC_CHECK_HEADERS([SDL.h], ,
AC_MSG_ERROR([ *** SDL library found but cannot find headers (http://www.libsdl.org/)]))

AC_CHECK_LIB([z], [inflate], ,
AC_MSG_ERROR([ *** Unable to find zlib (http://www.gzip.org/zlib/)]))

AC_CHECK_LIB([physfs], [PHYSFS_init], ,
AC_MSG_ERROR([ *** Unable to find PhysFS library (http://icculus.org/physfs/)]))
AC_CHECK_HEADERS([physfs.h], ,
AC_MSG_ERROR([ *** PhysFS library found but cannot find headers (http://icculus.org/physfs/)]))

if test -n "$CURL_CONFIG"; then
    LIBS="$LIBS `$CURL_CONFIG --libs`"
    CPPFLAGS="$CPPFLAGS `$CURL_CONFIG --cflags`"
fi
AC_CHECK_LIB([curl], [curl_global_init], ,
AC_MSG_ERROR([ *** Unable to find CURL library (http://curl.haxx.se/)]))
AC_CHECK_HEADERS([curl/curl.h], ,
AC_MSG_ERROR([ *** CURL library found but cannot find headers (http://curl.haxx.se/)]))

if test -n "$PKG_CONFIG"; then
    LIBS="$LIBS `$PKG_CONFIG --libs libxml-2.0`"
    CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags libxml-2.0`"
fi
AC_CHECK_LIB([xml2], [xmlInitParser], ,
AC_MSG_ERROR([ *** Unable to find libxml2 library (http://xmlsoft.org/)]))
AC_CHECK_HEADERS([libxml/xmlreader.h], ,
AC_MSG_ERROR([ *** libxml2 library found but cannot find headers (http://xmlsoft.org/)]))

AC_CHECK_LIB(png, png_write_info, ,
AC_MSG_ERROR([ *** Unable to find png library]))

AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, ,
AC_MSG_ERROR([ *** Unable to find SDL_image library with PNG support
(http://www.libsdl.org/projects/SDL_image/)]))

AC_CHECK_LIB(SDL_ttf, TTF_Quit, ,
AC_MSG_ERROR([ *** Unable to find SDL_ttf library (http://www.libsdl.org/projects/SDL_ttf/)]))

AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio], ,
AC_MSG_ERROR([ *** Unable to find SDL_mixer library (http://www.libsdl.org/projects/SDL_mixer/)]))

AC_CHECK_LIB([guichan_sdl], [gcnSDL], ,
AC_MSG_ERROR([ *** Unable to find Guichan SDL library (http://guichan.sf.net/)]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Option to enable OpenGL
AC_ARG_WITH(opengl,[  --without-opengl        don't use OpenGL ] )
if test "x$with_opengl" == "xno"; then
    with_opengl=no
else
    with_opengl=yes
    AC_CHECK_LIB([GL], [glBegin], ,
    AC_MSG_ERROR([ *** Unable to find OpenGL library]))
    AC_DEFINE(USE_OPENGL, 1, [Defines if tmw should use an OpenGL display])
fi

# Enable either tmwserv or eAthena
AC_ARG_WITH(
    [server],
    AS_HELP_STRING(
        [--with-server=ARG],
        [which server to use [[ARG=tmwserv,eathena]]
        [(default=eathena)]]
    ),
    [if test "$withval" = "yes"; then
        # default is eathena
        with_server="eathena"
     elif test "$withval" = "no"; then
        AC_MSG_ERROR([$PACKAGE_NAME cannot run without a server.])
     else
         with_server="$withval"
     fi],
    [with_server="eathena"]
)

if test "$with_server" = "tmwserv"; then
    AC_CHECK_LIB([enet], [enet_initialize], ,
    AC_MSG_ERROR([ *** Unable to find enet library (http://enet.bespin.org/)]))
    AC_DEFINE(TMWSERV_SUPPORT, 1, [Defines if tmwserv support is enabled])
elif test "$with_server" = "eathena"; then
    AC_CHECK_LIB(SDL_net, SDLNet_Init, ,
    AC_MSG_ERROR([ *** Unable to find SDL_net library]))
    AC_DEFINE(EATHENA_SUPPORT, 1, [Defines if eAthena support is enabled])
else
    AC_MSG_ERROR([unknown server: $with_server])
fi

AM_CONDITIONAL(SERVER_TMWSERV, test "$with_server" = "tmwserv")
AM_CONDITIONAL(SERVER_EATHENA, test "$with_server" = "eathena")

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/fonts/Makefile
data/graphics/Makefile
data/graphics/gui/Makefile
data/graphics/images/Makefile
data/help/Makefile
data/icons/Makefile
docs/Makefile
po/Makefile.in
])

AC_OUTPUT

echo
echo "Build with OpenGL: $with_opengl"
echo "Supported server:  $with_server"
echo
echo "configure complete, now type \"make\""
echo
