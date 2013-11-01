#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)

m4_define([required_lttoolbox_version], [3.2.0])
m4_define([required_libxml_version], [2.6.17])
m4_define([required_libpcre_version], [6.4])
#m4_define([required_pkg_config_version], [0.15])

AC_INIT([apertium], [3.2.0], [sortiz@users.sourceforge.net])
AC_CONFIG_HEADER([apertium/apertium_config.h])

AC_CANONICAL_SYSTEM

GENERIC_LIBRARY_NAME=apertium

# Release versioning
GENERIC_MAJOR_VERSION=3
GENERIC_MINOR_VERSION=2
GENERIC_MICRO_VERSION=0

# API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_API_VERSION)
AC_SUBST(GENERIC_MAJOR_VERSION)

# Shared library versioning
GENERIC_LIBRARY_VERSION=0:0:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed

AC_SUBST(GENERIC_LIBRARY_VERSION)
PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

VERSION=$GENERIC_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

# Checks for programs.

AC_MSG_CHECKING([Compilation architecture: PPC, i686, x86_64, Other])
if test x$(which arch) = x
then ARCH=$($(which uname) -m)
else ARCH=$($(which arch))
fi

case "${target_os}" in
  mingw*)
    MinGW=yes
  ;;
  cygwin*)
    Cygwin=yes
  ;;
esac

if test x$ARCH = xppc
then
  AC_MSG_RESULT([PowerPC])
  CFLAGS="$CFLAGS -Wall -ansi -fomit-frame-pointer"
  CXXFLAGS="$CXXFLAGS -Wall -ansi -fomit-frame-pointer"
else
  if test x$MinGW = xyes
  then 
    AC_MSG_RESULT([MinGW])
    CFLAGS="$CFLAGS -Wall -march=i686 -O3 -DMINGW -fomit-frame-pointer \
	        -funroll-loops -I/include -L/lib"
    CXXFLAGS="$CXXFLAGS -Wall -march=i686 -O3 \
            -fomit-frame-pointer -funroll-loops -I/include -L/lib"
    CPPFLAGS="$CPPFLAGS -I/include"
  else
    if test x$Cygwin = xyes
    then 
      AC_MSG_RESULT([Cygwin])
      CFLAGS="$CFLAGS -Wall -march=i686 -O3 -fomit-frame-pointer \
           -funroll-loops"
      CXXFLAGS="$CXXFLAGS -Wall -march=i686 -O3 \
              -fomit-frame-pointer -funroll-loops"
      CPPFLAGS="$CPPFLAGS"
    else
      if test x$ARCH = xi686
      then
        AC_MSG_RESULT([i686])
        CFLAGS="$CFLAGS -Wall -ansi -march=i686 -O3 -fomit-frame-pointer -funroll-loops"
        CXXFLAGS="$CXXFLAGS -Wall -ansi -march=i686 -O3 \
                -fomit-frame-pointer -funroll-loops"
      else
        if test x$ARCH = xx86_64
        then
          AC_MSG_RESULT([x86_64])
          CFLAGS="-Wall -ansi -O3 -mtune=nocona -fomit-frame-pointer -funroll-loops $CFLAGS"
          CXXFLAGS="-Wall -ansi -O3 -mtune=nocona \
                  -fomit-frame-pointer -funroll-loops $CXXFLAGS"
        else
          AC_MSG_RESULT([Other])
          CFLAGS="-Wall -ansi -O3 $CFLAGS"
          CXXFLAGS="-Wall -ansi -O3 $CXXFLAGS"
        fi
      fi
    fi    
  fi
fi

AC_PROG_CXX
AC_PROG_LIBTOOL
AM_SANITY_CHECK
AC_LANG_CPLUSPLUS

AC_ARG_ENABLE(debug,
              [  --enable-debug    Enable "-g -Wall" compiler options], 
              [CXXFLAGS="-g -Wall"; CFLAGS="-g -Wall"])

AC_ARG_ENABLE(profile,
              [  --enable-profile  Enable "-pg -g -Wall" compiler options],
              [CXXFLAGS="-pg -g -Wall"; CFLAGS="-pg -g -Wall"; LDFLAGS="-pg"])

AC_PATH_PROG(XMLLINT, xmllint, no)
if test x$ac_cv_path_XMLLINT = x
then
  AC_MSG_ERROR([You don't have xmllint installed.])
fi
if test x$ac_cv_path_XMLLINT = xno
then
  AC_MSG_ERROR([You don't have xmllint installed.])
fi
 
  AC_PATH_PROG(XSLTPROC, xsltproc, no)
  if test x$ac_cv_path_XSLTPROC = x
  then
    AC_MSG_ERROR([You don't have xsltproc installed.])
  fi
  if test x$ac_cv_path_XSLTPROC = xno
  then
    AC_MSG_ERROR([You don't have xsltproc installed.])
  fi

AC_PATH_PROG(BASH, bash, no)
if test x$ac_cv_path_BASH = x
then
  AC_MSG_ERROR([You don't have bash installed.])
fi
if test x$ac_cv_path_BASH = xno
then
  AC_MSG_ERROR([You don't have bash installed.])
fi

AC_PATH_PROG(FLEX, flex, no)
if test x$ac_cv_path_FLEX = x
then
  AC_MSG_ERROR([You don't have flex installed.])
fi
if test x$ac_cv_path_FLEX = xno
then
  AC_MSG_ERROR([You don't have flex installed.])
fi

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$ac_cv_path_PKG_CONFIG = x
then
  AC_MSG_ERROR([You don't have pkg-config installed.])
fi
if test x$ac_cv_path_PKG_CONFIG = xno
then
  AC_MSG_ERROR([You don't have pkg-config installed.])
fi

AC_CHECK_FUNCS(strcasecmp)

if test x$(uname) != xDarwin;
then 
AC_CHECK_HEADER(pcreposix.h,
  AC_CHECK_LIB(pcre, pcre_fullinfo,[
    LIBS="$LIBS -lpcreposix -lpcre"
    no_comp_check=yes],
    AC_CHECK_LIB(pcre, pcre_info,[ # info is deprecated in favor of fullinfo, but leave it in case old versions don't have fullinfo
      LIBS="$LIBS -lpcreposix -lpcre"
      no_comp_check=yes],
      AC_MSG_ERROR([*** unable to locate pcre library ***]))),
  AC_MSG_ERROR([*** unable to locate pcreposix.h include file ***]))

AC_CHECK_HEADER(pcrecpp.h,
  AC_CHECK_LIB(pcrecpp,pcre_compile,[
  LIBS="$LIBS -lpcrecpp"
  no_comp_check=yes],
  AC_MSG_ERROR([*** unable to locate pcrecpp library ***])),
  AC_MSG_ERROR([*** unable to locate pcrecpp.h include file ***])) 
fi


PKG_CHECK_MODULES(APERTIUM, [
  lttoolbox >= required_lttoolbox_version
  libxml-2.0 >= required_libxml_version 
  libpcre >= required_libpcre_version], CPPFLAGS="$CPPFLAGS $APERTIUM_CFLAGS"; LIBS="$LIBS $APERTIUM_LIBS")

# Check for wide strings
AC_DEFUN([AC_CXX_WSTRING],[
  AC_CACHE_CHECK(whether the compiler supports wide strings,
  ac_cv_cxx_wstring,
  [AC_LANG_SAVE
   AC_LANG_CPLUSPLUS
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <string>]],[[
std::wstring test = L"test";
   ]])],
   [ac_cv_cxx_wstring=yes], [ac_cv_cxx_wstring=no])
   AC_LANG_RESTORE
  ])
])

AC_CXX_WSTRING
AC_C_BIGENDIAN

if test "$ac_cv_cxx_wstring" = no 
then
  AC_MSG_ERROR([Missing wide string support])
fi

  
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h stddef.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC

AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked, getopt_long, fgetwc_unlocked, fputwc_unlocked, fgetws_unlocked, fputws_unlocked])

AC_CHECK_FUNCS([setlocale strdup getopt_long snprintf])

AC_OUTPUT([Makefile apertium.pc apertium/Makefile])
