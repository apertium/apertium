AC_PREREQ(2.52)

m4_define([PKG_VERSION_MAJOR], [3])
m4_define([PKG_VERSION_MINOR], [9])
m4_define([PKG_VERSION_PATCH], [12])

# Bump if the ABI (not API) changed in a backwards-incompatible manner
m4_define([PKG_VERSION_ABI], [3])

AC_INIT([apertium], [PKG_VERSION_MAJOR.PKG_VERSION_MINOR.PKG_VERSION_PATCH], [apertium-stuff@lists.sourceforge.net], [apertium], [https://wiki.apertium.org/])
AC_CONFIG_HEADER([apertium/apertium_config.h])

VERSION=$PACKAGE_VERSION
VERSION_ABI=PKG_VERSION_ABI

AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(VERSION_ABI)

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

PKG_CHECK_MODULES(LTTOOLBOX, [lttoolbox >= 3.7.2], CPPFLAGS="$CPPFLAGS $LTTOOLBOX_CFLAGS"; LIBS="$LIBS $LTTOOLBOX_LIBS")
PKG_CHECK_MODULES(LIBXML2, [libxml-2.0 >= 2.6.17], CPPFLAGS="$CPPFLAGS $LIBXML2_CFLAGS"; LIBS="$LIBS $LIBXML2_LIBS")
PKG_CHECK_MODULES(ICU, [icu-i18n, icu-io, icu-uc], CPPFLAGS="$CPPFLAGS $ICU_CFLAGS"; LIBS="$LIBS $ICU_LIBS")

AC_C_BIGENDIAN

# Checks for header files.
AC_LANG(C++)
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h stddef.h filesystem string_view])
AC_CHECK_LIB([stdc++fs], [_ZNSt12experimental10filesystem2v112current_pathEv])
AC_CHECK_HEADER([utf8cpp/utf8.h], [CPPFLAGS="-I/usr/include/utf8cpp/ $CPPFLAGS"], [
  AC_CHECK_HEADER([utf8.h], [], [AC_MSG_ERROR([You don't have utfcpp installed.])])
])

AC_CHECK_DECLS([fread_unlocked, fwrite_unlocked, fgetc_unlocked, fputc_unlocked, fputs_unlocked, getopt, getopt_long])
AC_CHECK_FUNCS([setlocale strdup getopt snprintf])
AC_REPLACE_FUNCS(getopt_long)

AM_CONDITIONAL([WINDOWS], [test x$version_type = xwindows])
#AS_IF([test x$version_type = xwindows], [AC_DEFINE(HAVE_GETOPT_LONG,0)], [])

# Checks for highest supported C++ standard
for version in 23 2b 20 2a 17; do
  version_flag="-std=c++${version}"
  AX_CHECK_COMPILE_FLAG([${version_flag}], [break], [version_flag=none])
done
AS_IF([test "$version_flag" = none], [
  AC_MSG_ERROR([Could not enable at least C++17 - upgrade your compiler])
])
CXXFLAGS="$CXXFLAGS ${version_flag}"

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
