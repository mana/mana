AC_PREREQ(2.59)
AC_INIT([The Mana World], [0.0.16], [umperio@users.sourceforge.net], [tmw])
AC_CONFIG_HEADERS([config.h:config.h.in])
AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries
AC_CHECK_LIB([guichan], [gcnGuichanVersion], ,
AC_MSG_ERROR([ *** Unable to find Guichan library (guichan.sf.net)]))

#AC_CHECK_LIB([guichan_sdl], [gcnSDL], ,
#AC_MSG_ERROR([ *** Unable to find Guichan SDL library (guichan.sf.net)]))

#AC_CHECK_LIB([guichan_opengl], [gcnOpenGL], ,
#AC_MSG_ERROR([ *** Unable to find Guichan OpenGL library (guichan.sf.net)]))

AC_CHECK_LIB([physfs], [PHYSFS_init], ,
AC_MSG_ERROR([ *** Unable to find PhysFS library (icculus.org/physfs/)]))

AC_CHECK_LIB([curl], [curl_global_init], ,
AC_MSG_ERROR([ *** Unable to find CURL library (http://curl.haxx.se/)]))
CURL_LIBS=" `curl-config --libs`"
CURL_CFLAGS=" `curl-config --cflags` "
AC_SUBST(CURL_LIBS)
AC_SUBST(CURL_CFLAGS)

AC_CHECK_LIB([xml2], [xmlInitParser], ,
AC_MSG_ERROR([ *** Unable to find libxml2 library (xmlsoft.org)]))

AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, ,
AC_MSG_ERROR([ *** Unable to find SDL_image library with PNG support
(http://www.libsdl.org/projects/SDL_image/)]))

AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio, ,
AC_MSG_ERROR([ *** Unable to find SDL_mixer library
(http://www.libsdl.org/projects/SDL_mixer/)]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Check for guichan headers
AC_CHECK_HEADERS([guichan.hpp], ,
AC_MSG_ERROR([*** Library found but cannot find headers (guichan.sf.net) *** ]))

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_HEADER_TIME
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit floor getcwd gethostbyname memset mkdir select socket])

AM_INIT_AUTOMAKE

# Option to enable OpenGL
AC_ARG_WITH(opengl,[  --with-opengl           use OpenGL ] )
if test "x$with_opengl" == "xyes"; then
    with_opengl=yes
    OPENGL_CFLAGS=' -DUSE_OPENGL'
    OPENGL_LIBS=' -lGL -lguichan_opengl'
    AC_SUBST(OPENGL_CFLAGS)
    AC_SUBST(OPENGL_LIBS)
else
    with_opengl=no
fi

# FIND_PATH(programm-name, variable-name, list of directories,
#     if-not-found, test-parameter)
AC_DEFUN([FIND_PATH],
[
   AC_MSG_CHECKING([for $1])
   if test -n "$$2"; then
        cv_path="$$2";
   else
        cache=`echo $1 | sed 'y%./+-%__p_%'`

        AC_CACHE_VAL(cv_path_cache,
        [
        cv_path="NONE"
        dirs="$3"
        save_IFS=$IFS
        IFS=':'
        for dir in $PATH; do
          dirs="$dirs $dir"
        done
        IFS=$save_IFS

        for dir in $dirs; do
          if test -x "$dir/$1"; then
            if test -n "$5"
            then
              evalstr="$dir/$1 $5 2>&1 "
              if eval $evalstr; then
                cv_path="$dir/$1"
                break
              fi
            else
                cv_path="$dir/$1"
                break
            fi
          fi
        done

        eval "cv_path_$cache=$cv_path"

        ])

      eval "cv_path=\"`echo '$cv_path_'$cache`\""

   fi

   if test -z "$cv_path" || test "$cv_path" = NONE; then
      AC_MSG_RESULT(not found)
      $4
   else
      AC_MSG_RESULT($cv_path)
      $2=$cv_path

   fi
])

FIND_PATH(sdl-config, LIBSDL_CONFIG, [${prefix}/bin ${exec_prefix}/bin /usr/local/bin /opt/local/bin], [
  AC_MSG_ERROR([Could not find libsdl anywhere, check http://www.sdl.org])
])

if test -n "$LIBSDL_CONFIG"; then
  LIBSDL_LIBS="`$LIBSDL_CONFIG --libs` -lSDL_image -lSDL_mixer -lpthread "
  LIBSDL_CFLAGS="`$LIBSDL_CONFIG --cflags`"

  AC_DEFINE_UNQUOTED(HAVE_LIBSDL, 1, [Defines if your system has the LIBSDL library])
else
  AC_DEFINE_UNQUOTED(HAVE_LIBSDL, 0, [Defines if your system has the LIBSDL library])
fi

AC_SUBST(LIBSDL_LIBS)
AC_SUBST(LIBSDL_CFLAGS)

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/graphics/Makefile
data/graphics/gui/Makefile
data/graphics/images/Makefile
data/graphics/sprites/Makefile
data/graphics/tiles/Makefile
data/help/Makefile
data/icons/Makefile
data/maps/Makefile
data/sfx/Makefile
docs/Makefile
])

AC_OUTPUT

echo
echo Build with OpenGL: $with_opengl
echo
echo configure complete, now type \"make\"
echo
