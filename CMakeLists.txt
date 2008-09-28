cmake_minimum_required(VERSION 2.6)
PROJECT (apertium-3.1)

SET (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

INCLUDE (CheckFunctionExists)
INCLUDE (CheckSymbolExists)

INCLUDE_DIRECTORIES (.)

IF (WIN32)
        SET(APERTIUM_PATH "$(dirname $BASH)")
        SET(LTTOOLBOX_PATH "$(dirname $BASH)")
        SET(DEFAULT_DIRECTORY "$(dirname $BASH)/../share/apertium")
ELSE (WIN32)
        # THIS IS NOT QUITE CORRECT FOR POSIX PLATFORMS.
        # TODO: Fix this
        SET(APERTIUM_PATH "${CMAKE_ROOT}/bin")
        SET(LTTOOLBOX_PATH "${CMAKE_ROOT}/bin")
        SET(DEFAULT_DIRECTORY "${CMAKE_ROOT}/share/apertium")
ENDIF (WIN32)

SET (SCRIPT_HEADER_FILE "${PROJECT_SOURCE_DIR}/apertium/script_header.sh.cmake_in")

IF (WIN32)
        SET (WIN32_DIR ${PROJECT_SOURCE_DIR}/apertium/win32)
        INCLUDE_DIRECTORIES (${WIN32_DIR})
        SET (EXTRA_SOURCES "${WIN32_DIR}/getopt.c" "${WIN32_DIR}/libgen.c")

        SET(FLEX_EXECUTABLE        C:/Program\ Files/GnuWin32/bin/flex.exe)

        SET(ICONV_BASE_DIR         C:/Program\ Files/iconv)
        SET(ICONV_INCLUDE_DIR      ${ICONV_BASE_DIR}/include)
	SET(ICONV_LIBRARIES        ${ICONV_BASE_DIR}/lib/iconv.lib)

        SET(LIBXML2_BASE_DIR       C:/Program\ Files/LibXMl)
        SET(LIBXML2_INCLUDE_DIR    ${LIBXML2_BASE_DIR}/include)
        SET(LIBXML2_LIBRARIES      ${LIBXML2_BASE_DIR}/lib/libxml2.lib)
        SET(XSLTPROC_EXECUTABLE    ${LIBXML2_BASE_DIR}/bin/xsltproc.exe)

        SET(LTTOOLBOX3_BASE_DIR    C:/Program\ Files/apertium-3.0)
        SET(LTTOOLBOX3_INCLUDE_DIR ${LTTOOLBOX3_BASE_DIR}/include/lttoolbox-3.0)
        SET(LTTOOLBOX3_LIBRARIES   ${LTTOOLBOX3_BASE_DIR}/lib/lttoolbox3.lib)

        SET(LIBPCRE_BASE_DIR       C:/Program\ Files/PCRE) 
        SET(LIBPCRE_INCLUDE_DIR    "${LIBPCRE_BASE_DIR}/include")
        SET(LIBPCRE_LIBRARIES      "${LIBPCRE_BASE_DIR}/lib/pcrecpp.lib"
                                   "${LIBPCRE_BASE_DIR}/lib/pcreposix.lib"
                                   "${LIBPCRE_BASE_DIR}/lib/pcre.lib")

        ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS /D STDC_HEADERS /D PCRE_STATIC)
ENDIF (WIN32)

INCLUDE (CMakeUseFlex)
INCLUDE (CMakeUseXsltproc)

FIND_PACKAGE (LibXml2    REQUIRED)
FIND_PACKAGE (Lttoolbox3 REQUIRED)
FIND_PACKAGE (LibPcre    REQUIRED)

SET (PACKAGE_BUGREPORT sortiz@users.sourceforge.net)
SET (PACKAGE_NAME      apertium/apertium.h)
SET (PACKAGE_STRING    apertium/apertium.h 3.1.0)
SET (PACKAGE_TARNAME   lttoolbox-lttoolbox-h)
SET (PACKAGE_VERSION   3.1.0)

MACRO (BOOL_CHECK_SYMBOL_EXISTS SYMBOL HEADER VAR)
        CHECK_SYMBOL_EXISTS (${SYMBOL} ${HEADER} ${VAR})
        IF (${VAR})
                SET (${VAR} "1")
        ELSE (${VAR})
                SET (${VAR} "0")
        ENDIF (${VAR})
ENDMACRO (BOOL_CHECK_SYMBOL_EXISTS)

BOOL_CHECK_SYMBOL_EXISTS (fputs_unlocked  stdio.h HAVE_DECL_FPUTS_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fgetc_unlocked  stdio.h HAVE_DECL_FGETC_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fputc_unlocked  stdio.h HAVE_DECL_FPUTC_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fwrite_unlocked stdio.h HAVE_DECL_FWRITE_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fread_unlocked  stdio.h HAVE_DECL_FREAD_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fgetwc_unlocked stdio.h HAVE_DECL_FGETWC_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fputwc_unlocked stdio.h HAVE_DECL_FPUTWC_UNLOCKED)
BOOL_CHECK_SYMBOL_EXISTS (fputws_unlocked stdio.h HAVE_DECL_FPUTWS_UNLOCKED)

CHECK_FUNCTION_EXISTS (setlocale HAVE_SETLOCALE)
CHECK_FUNCTION_EXISTS (strdup HAVE_STRDUP)
CHECK_FUNCTION_EXISTS (getopt_long HAVE_GETOPT_LONG)

CONFIGURE_FILE (${PROJECT_SOURCE_DIR}/apertium/apertium_config.h.cmake_in ${PROJECT_SOURCE_DIR}/apertium/apertium_config.h)

ADD_SUBDIRECTORY (apertium)
