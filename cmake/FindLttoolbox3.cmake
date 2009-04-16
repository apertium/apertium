# - Try to find Lttoolbox3
# Once done this will define
#
#  LTTOOLBOX3_FOUND - system has Lttoolbox3
#  LTTOOLBOX3_INCLUDE_DIR - the Lttoolbox3 include directory
#  LTTOOLBOX3_LIBRARIES - the libraries needed to use Lttoolbox3
#  LTTOOLBOX3_DEFINITIONS - Compiler switches required for using Lttoolbox3
#
# Copyright (c) 2006, Alexander Neundorf <neundorf@kde.org>
# This code is available under the BSD license, see licenses/BSD for details.

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# This is derived from FindLibXml2.cmake

IF (LTTOOLBOX3_INCLUDE_DIR AND LTTOOLBOX3_LIBRARIES)
   # in cache already
   SET(Lttoolbox3_FIND_QUIETLY TRUE)
ENDIF (LTTOOLBOX3_INCLUDE_DIR AND LTTOOLBOX3_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   INCLUDE(UsePkgConfig)
   PKGCONFIG(lttoolbox-3.0 LTTOOLBOX3_INCLUDES LTTOOLBOX3_LIB_DIR LTTOOLBOX3_LDFLAGS LTTOOLBOX3_CFLAGS)
   SET(LTTOOLBOX3_DEFINITIONS ${LTTOOLBOX3_CFLAGS})
ENDIF (NOT WIN32)

FIND_PATH(LTTOOLBOX3_INCLUDE_DIR lttoolbox/alphabet.h
   PATHS ${LTTOOLBOX3_INCLUDES}
   PATH_SUFFIXES lttoolbox-3.0)

FIND_LIBRARY(LTTOOLBOX3_LIBRARIES
             NAMES lttoolbox3
             PATHS ${LTTOOLBOX3_LIB_DIR})

IF (LTTOOLBOX3_INCLUDE_DIR AND LTTOOLBOX3_LIBRARIES)
   SET(LTTOOLBOX3_FOUND TRUE)
ELSE (LTTOOLBOX3_INCLUDE_DIR AND LTTOOLBOX3_LIBRARIES)
   SET(LTTOOLBOX3_FOUND FALSE)
ENDIF (LTTOOLBOX3_INCLUDE_DIR AND LTTOOLBOX3_LIBRARIES)

IF (LTTOOLBOX3_FOUND)
   IF (NOT Lttoolbox3_FIND_QUIETLY)
      MESSAGE(STATUS "Found Lttoolbox3: ${LTTOOLBOX3_LIBRARIES}")
   ENDIF (NOT Lttoolbox3_FIND_QUIETLY)
ELSE (LTTOOLBOX3_FOUND)
   IF (Lttoolbox3_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find Lttoolbox3")
   ENDIF (Lttoolbox3_FIND_REQUIRED)
ENDIF (LTTOOLBOX3_FOUND)

MARK_AS_ADVANCED(LTTOOLBOX3_INCLUDE_DIR LTTOOLBOX3_LIBRARIES)

