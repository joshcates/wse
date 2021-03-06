# QWT_FOUND       = system has QWT lib
#
# QWT_LIBRARY     = full path to the QWT library
#
# QWT_INCLUDE_DIR      = where to find headers 
#

FIND_PATH(QWT_INCLUDE_DIR qwt.h 
  /usr/include
  /usr/include/qwt-qt4
  /usr/local/include
  "$ENV{LIB_DIR}/include" 
  "$ENV{LIB_DIR}/include/qwt"
  )
FIND_LIBRARY(QWT_LIBRARY qwt PATHS 
  /usr/lib
  /usr/local/lib
  "$ENV{LIB_DIR}/lib" 
  )
IF (NOT QWT_LIBRARY)
  # try using ubuntu lib naming
  FIND_LIBRARY(QWT_LIBRARY qwt-qt4 PATHS 
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib" 
    )
ENDIF (NOT QWT_LIBRARY)

IF (QWT_INCLUDE_DIR AND QWT_LIBRARY)
  SET(QWT_FOUND TRUE)
ENDIF (QWT_INCLUDE_DIR AND QWT_LIBRARY)

IF (QWT_FOUND)
  IF (NOT QWT_FIND_QUIETLY)
    MESSAGE(STATUS "Found QWT: ${QWT_LIBRARY}")
  ENDIF (NOT QWT_FIND_QUIETLY)
ELSE (QWT_FOUND)
  IF (QWT_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find QWT")
  ENDIF (QWT_FIND_REQUIRED)
ENDIF (QWT_FOUND)
