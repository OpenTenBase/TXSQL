
MACRO (MYSQL_CHECK_TENCENTSM)

    SET(TencentSMRoot ${WITH_TSM}/lite)
    SET(TencentSMLibName "libTencentSM.a")

    INCLUDE_DIRECTORIES(SYSTEM ${TencentSMRoot}/source/include)

    IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        SET(TencentSM_LIBRARY ${TencentSMRoot}/release/linux64/Release/${TencentSMLibName})
    ELSE()
        SET(TencentSM_LIBRARY ${TencentSMRoot}/release/linuxarm64/Release/${TencentSMLibName})
    ENDIF()

ENDMACRO()