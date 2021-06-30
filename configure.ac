AC_PREREQ(2.52)

m4_define([PKG_VERSION_MAJOR], [3])
m4_define([PKG_VERSION_MINOR], [8])
m4_define([PKG_VERSION_PATCH], [0])

AC_INIT([apertium], [PKG_VERSION_MAJOR.PKG_VERSION_MINOR.PKG_VERSION_PATCH], [apertium-stuff@lists.sourceforge.net], [apertium], [https://wiki.apertium.org/])
AC_CONFIG_HEADER([apertium/apertium_config.h])

VERSION=$PACKAGE_VERSION
VERSION_MAJOR=PKG_VERSION_MAJOR
VERSION_API=PKG_VERSION_MAJOR.PKG_VERSION_MINOR
SOVERSION=1:0:0

AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_API)
AC_SUBST(SOVERSION)

AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_LIBTOOL
AC_LANG_CPLUSPLUS

CFLAGS="-Wall -Wextra $CFLAGS"
CXXFLAGS="-Wall -Wextra $CXXFLAGS"

AC_ARG_ENABLE(debug,
              [  --enable-debug    Enable "-g -Wall" compiler options],
              [CXXFLAGS="-g -Wall"; CFLAGS="-g -Wall"; AC_DEFINE([ENABLE_DEBUG], [1], [ENABLE_DEBUG])])

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

AC_PATH_PROG(FLEX, flex, no)
if test x$ac_cv_path_FLEX = x
then
  AC_MSG_ERROR([You don't have flex installed.])
fi
if test x$ac_cv_path_FLEX = xno
then
  AC_MSG_ERROR([You don't have flex installed.])
fi

AC_CHECK_FUNCS(strcasecmp)

PKG_CHECK_MODULES(LTTOOLBOX, [lttoolbox >= 3.6.0], CPPFLAGS="$CPPFLAGS $LTTOOLBOX_CFLAGS"; LIBS="$LIBS $LTTOOLBOX_LIBS")
PKG_CHECK_MODULES(LIBXML2, [libxml-2.0 >= 2.6.17], CPPFLAGS="$CPPFLAGS $LIBXML2_CFLAGS"; LIBS="$LIBS $LIBXML2_LIBS")
PKG_CHECK_MODULES(ICU, [icu-i18n, icu-io, icu-uc], CPPFLAGS="$CPPFLAGS $ICU_CFLAGS"; LIBS="$LIBS $ICU_LIBS")

AC_C_BIGENDIAN

# Checks for header files.
AC_LANG(C++)
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h stddef.h filesystem string_view])
AC_CHECK_LIB([stdc++fs], [_ZNSt12experimental10filesystem2v112current_pathEv])
AC_CHECK_HEADER([utf8.h], [], [AC_MSG_ERROR([You don't have utfcpp installed.])])

AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked, getopt, getopt_long])
AC_CHECK_FUNCS([setlocale strdup getopt snprintf])
AC_REPLACE_FUNCS(getopt_long)

AM_CONDITIONAL([WINDOWS], [test x$version_type = xwindows])
#AS_IF([test x$version_type = xwindows], [AC_DEFINE(HAVE_GETOPT_LONG,0)], [])

# Checks for highest supported C++ standard
AX_CHECK_COMPILE_FLAG([-std=c++20], [CXXFLAGS="$CXXFLAGS -std=c++20"], [
 AX_CHECK_COMPILE_FLAG([-std=c++2a], [CXXFLAGS="$CXXFLAGS -std=c++2a"], [
  AX_CHECK_COMPILE_FLAG([-std=c++17], [CXXFLAGS="$CXXFLAGS -std=c++17"], [
   AX_CHECK_COMPILE_FLAG([-std=c++1z], [CXXFLAGS="$CXXFLAGS -std=c++1z"], [
    AX_CHECK_COMPILE_FLAG([-std=c++14], [CXXFLAGS="$CXXFLAGS -std=c++14"], [
     AX_CHECK_COMPILE_FLAG([-std=c++1y], [CXXFLAGS="$CXXFLAGS -std=c++1y"], [])
    ])
   ])
  ])
 ])
])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <cstddef>
#include <cstdint>
#include <type_traits>
]],[[
using namespace std;
static_assert(!is_same<size_t,uint32_t>::value, "size_t == uint32_t");
static_assert(!is_same<size_t,uint64_t>::value, "size_t == uint64_t");
   ]])], [AC_DEFINE([SIZET_NOT_CSTDINT], [1], [size_t != (uint32_t, uint64_t)])])

AC_CONFIG_FILES([python/setup.py])

AC_SUBST(APERTIUM_CFLAGS)
AC_SUBST(LIBS)

AM_PATH_PYTHON([3.4], [], [AC_MSG_WARN([Can't generate SWIG wrapper or run tests without Python])])

AC_ARG_ENABLE([python-bindings],
              AS_HELP_STRING([--enable-python-bindings],
                             [build python bindings (default=disabled)]),
              [enable_python_bindings=$enableval],
              [enable_python_bindings=no])
AM_CONDITIONAL([HAVE_PYTHON_BINDINGS], [test x$enable_python_bindings = xyes])

AC_ARG_VAR([PYTHON_INSTALL_PARAMS], [Parameters to pass to the Python 3 module install step])
if test "x$PYTHON_INSTALL_PARAMS" = "x"
then
  PYTHON_INSTALL_PARAMS="--prefix=\$(prefix) --root=\$(DESTDIR)/"
fi

AC_OUTPUT([
	Makefile apertium.pc
	apertium/Makefile
	scripts/Makefile
	tests/Makefile tests/tagger/Makefile
	python/Makefile
	])
