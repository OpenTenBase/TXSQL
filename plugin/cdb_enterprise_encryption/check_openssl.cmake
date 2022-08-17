FUNCTION (MYSQL_CHECK_OPENSSL)
  IF(NOT OPENSSL_INCLUDE_DIR AND
     NOT OPENSSL_LIBRARY   AND
     NOT CRYPTO_LIBRARY
    )

    FIND_PATH(OPENSSL_ROOT_DIR
      NAMES include/openssl/ssl.h
      HINTS /usr/local /usr/local/ssl
    )

    FIND_PATH(OPENSSL_INCLUDE_DIR
      NAMES openssl/ssl.h
      HINTS ${OPENSSL_ROOT_DIR}/include
    )

    # Prefer .a over .so
    IF(NOT WITH_XTRABACKUP)
      LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    ENDIF()

    # Find only static libraries
    FIND_LIBRARY(OPENSSL_LIBRARY
      NAMES ssl
      HINTS ${OPENSSL_ROOT_DIR}/lib /usr/lib /usr/lib64)
    FIND_LIBRARY(CRYPTO_LIBRARY
      NAMES crypto
      HINTS ${OPENSSL_ROOT_DIR}/lib /usr/lib /usr/lib64)

    IF(NOT WITH_XTRABACKUP)
      LIST(REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    ENDIF()

    # Verify version number. Version information looks like:
    #   #define OPENSSL_VERSION_NUMBER 0x1000103fL
    # Encoded as MNNFFPPS: major minor fix patch status
    FILE(STRINGS "${OPENSSL_INCLUDE_DIR}/openssl/opensslv.h"
      OPENSSL_VERSION_NUMBER
      REGEX "^#[ ]*define[\t ]+OPENSSL_VERSION_NUMBER[\t ]+0x[0-9].*"
    )
    STRING(REGEX REPLACE
      "^.*OPENSSL_VERSION_NUMBER[\t ]+0x([0-9]).*$" "\\1"
      OPENSSL_MAJOR_VERSION "${OPENSSL_VERSION_NUMBER}"
    )

    IF(OPENSSL_INCLUDE_DIR AND
       OPENSSL_LIBRARY     AND
       CRYPTO_LIBRARY      AND
       OPENSSL_MAJOR_VERSION STREQUAL "1"
      )
      SET(OPENSSL_FOUND TRUE)
      MESSAGE(STATUS "OPENSSL_INCLUDE_DIR = ${OPENSSL_INCLUDE_DIR}")
      MESSAGE(STATUS "OPENSSL_LIBRARY = ${OPENSSL_LIBRARY}")
      MESSAGE(STATUS "CRYPTO_LIBRARY = ${CRYPTO_LIBRARY}")
      MESSAGE(STATUS "OPENSSL_MAJOR_VERSION = ${OPENSSL_MAJOR_VERSION}")
    ELSE()
      SET(OPENSSL_FOUND FALSE)
      MESSAGE(SEND_ERROR
        "Cannot find appropriate system libraries for openssl. ")
    ENDIF()
  ENDIF()
ENDFUNCTION()

