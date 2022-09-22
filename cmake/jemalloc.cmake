INCLUDE(ExternalProject)

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  SET(AARCH64_OPT "--with-lg-hugepage=21")
ENDIF()

MACRO (USE_BUNDLED_JEMALLOC)
  SET(SOURCE_DIR "${CMAKE_SOURCE_DIR}/extra/jemalloc/jemalloc-5.2.1")
  SET(BINARY_DIR "${CMAKE_BINARY_DIR}/jemalloc/jemalloc-5.2.1/build")
  SET(LIBJEMALLOC "libjemalloc")
  SET(JEMALLOC_CONFIGURE_OPTS "CC=${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}" "--with-malloc-conf=background_thread:true,dirty_decay_ms:10000,muzzy_decay_ms:0" "--with-private-namespace=jemalloc_internal_" "--enable-cc-silence" ${AARCH64_OPT})
  IF (CMAKE_BUILD_TYPE MATCHES "Debug" AND NOT APPLE) # see the comment in CMakeLists.txt
    LIST(APPEND JEMALLOC_CONFIGURE_OPTS --enable-debug)
  ENDIF()


  IF(CMAKE_GENERATOR MATCHES "Makefiles")
    SET(MAKE_COMMAND ${CMAKE_MAKE_PROGRAM})
  ELSE() # Xcode/Ninja generators
    SET(MAKE_COMMAND make)
  ENDIF()

  ExternalProject_Add(jemalloc
    PREFIX extra/jemalloc/jemalloc-5.2.1 
    SOURCE_DIR ${SOURCE_DIR}
    BINARY_DIR ${BINARY_DIR}
    STAMP_DIR  ${BINARY_DIR}
    CONFIGURE_COMMAND "${SOURCE_DIR}/configure" ${JEMALLOC_CONFIGURE_OPTS}
    BUILD_COMMAND  ${MAKE_COMMAND} "build_lib_static"
    INSTALL_COMMAND ""
  )
  MESSAGE("jemalloc ${SOURCE_DIR}")
  MESSAGE("jemalloc binary ${BINARY_DIR}")
  ADD_LIBRARY(libjemalloc STATIC IMPORTED)
  SET_TARGET_PROPERTIES(libjemalloc PROPERTIES IMPORTED_LOCATION "${BINARY_DIR}/lib/libjemalloc_pic.a")
  ADD_DEPENDENCIES(jemalloc libjemalloc)
ENDMACRO()

IF(CMAKE_SYSTEM_NAME MATCHES "Linux" OR APPLE)
 # Linux and OSX are the only systems where bundled jemalloc can be built without problems,
 # as they both have GNU make and jemalloc actually compiles.
 # Also, BSDs use jemalloc as malloc already
 SET(WITH_JEMALLOC_DEFAULT "yes")
ELSE()
 SET(WITH_JEMALLOC_DEFAULT "no")
ENDIF()

# SET(WITH_JEMALLOC ${WITH_JEMALLOC_DEFAULT} CACHE STRING
#    "Which jemalloc to use. Possible values are 'no', 'bundled', 'system', 'yes' (system if possible, otherwise bundled)")

MACRO (MYSQL_CHECK_JEMALLOC)
  USE_BUNDLED_JEMALLOC()
  SET(MALLOC_LIBRARY "bundled jemalloc")
ENDMACRO()

