cmake_minimum_required(VERSION 2.8)

project(TencentSM)
aux_source_directory(. DIR_LIB_SRCS)

#info
message("----------${PROJECT_NAME}------------")
message("CMAKE_MODULE_PATH:${CMAKE_MODULE_PATH}")
message("CMAKE_TencentSM_LIBTYPE:${CMAKE_TencentSM_LIBTYPE}")

#set var for all platform
set(MY_C_FLAGS "-std=c99")
set(HEAHDER_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/include/)
set(MY_PROJECT_LINKDIR ${SUB_LIBRARY_OUTPUT_PATH})
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-D_DEBUG)
else()
    remove_definitions(-D_DEBUG)
ENDIF()
IF (CMAKE_TencentSM_LIBTYPE MATCHES "static")
	set(TencentSM_LIBTYPE STATIC)
ELSEIF (CMAKE_TencentSM_LIBTYPE MATCHES "shared")
	set(TencentSM_LIBTYPE SHARED)
ENDIF()
# copy header file to include directory
configure_file(${PROJECT_SOURCE_DIR}/include/sm.h ${HEAHDER_OUTPUT_PATH} COPYONLY)
#include path
set(MY_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/../include ${PROJECT_SOURCE_DIR}/../lib/include)

#subdir 子目录的顺序很重要，按从上层到下层排，被依赖的放后面，否则可能出现编译动态库出现U(未定义)型函数接口
set(SUBDIR_NAMES  sm2 sm4 hmac sm3 asn1 common)
foreach(subdir ${SUBDIR_NAMES})
	macro_set_subdir(${subdir})
endforeach(subdir)
message("HEAHDER_OUTPUT_PATH:${HEAHDER_OUTPUT_PATH}")
message("MY_ALLSUBLIBS:${MY_ALLSUBLIBS}")
message("CMAKE_SYSTEM_NAME:${CMAKE_SYSTEM_NAME}")
#↓platform features↓
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	include_directories(${MY_INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/../gmp/include)
    message(STATUS "current platform: Linux ")
	IF (CMAKE_TencentSM_ARCH MATCHES "linux32")
		message(STATUS "ARCH: Linux32 ")
		set(TencentSM_ARCH linux32)
	ELSEIF (CMAKE_TencentSM_ARCH MATCHES "linux64")
		message(STATUS "ARCH: Linux64 ")
        set(TencentSM_ARCH linux64)
        set (ASM_SOURCES
                ${PROJECT_SOURCE_DIR}/sm2/ecp_sm2z256-x86_64.s
        )
    ELSE()
        set(TencentSM_ARCH ${CMAKE_TencentSM_ARCH})
	ENDIF()

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_C_FLAGS} ")

	# enable_language(C ASM)

    # for use fmemopen with __USE_XOPEN2K8
    add_definitions(-D_GNU_SOURCE)

	# 开启fPIC选项
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/release/${TencentSM_ARCH}/${CMAKE_BUILD_TYPE})

	link_directories(${CMAKE_SOURCE_DIR}/gmp/lib/${TencentSM_ARCH} ${CMAKE_SOURCE_DIR}/libtasn1/product/lib ${MY_PROJECT_LINKDIR})

	add_library (${PROJECT_NAME} ${TencentSM_LIBTYPE} ${DIR_LIB_SRCS} ${ASM_SOURCES})

	IF (CMAKE_TencentSM_LIBTYPE MATCHES "static")
		target_link_libraries(${PROJECT_NAME} ${MY_ALLSUBLIBS} libgmp.a)
	ELSEIF (CMAKE_TencentSM_LIBTYPE MATCHES "shared")
		target_link_libraries(${PROJECT_NAME} -Wl,--whole-archive ${MY_ALLSUBLIBS} -Wl,--no-whole-archive libgmp.a libtasn1.a)
	ENDIF()

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	message(STATUS "current platform: Windows")
    set(MY_LINK_OBJSUFFIX ".obj")
	#../build/windows/include来自jdk1.8.0，win32和win64定义在jdk1.8是一样的,主要是jni.h和jni_md.h, 而gmp.h来自mingw的路径
	include_directories(${MY_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/build/windows/include ${CMAKE_SOURCE_DIR}/build/windows/include/win32)

	IF (CMAKE_TencentSM_ARCH MATCHES "win32")
		set(TencentSM_ARCH win32)
	ELSEIF (CMAKE_TencentSM_ARCH MATCHES "win64")
		set(TencentSM_ARCH win64)
	ENDIF()

	set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/release/win/${TencentSM_ARCH}/${CMAKE_BUILD_TYPE})

    IF (CMAKE_TencentSM_LIBTYPE MATCHES "shared")
        Add_Definitions(-DSMLib_EXPORTS)
        Add_Definitions(-DJNI_INTERFACE)
    ELSE ()
        Add_Definitions(-D_WIN_STATIC)
    ENDIF ()

	link_directories(${MY_PROJECT_LINKDIR} ${CMAKE_SOURCE_DIR}/libtasn1/product/lib  ${CMAKE_SOURCE_DIR}/gmp/lib/${TencentSM_ARCH})

	add_library (${PROJECT_NAME} ${TencentSM_LIBTYPE} ${DIR_LIB_SRCS})
	target_link_libraries(${PROJECT_NAME} -Wl,--whole-archive ${MY_ALLSUBLIBS} -Wl,--no-whole-archive libgmp.a libws2_32.a libtasn1.a)

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	include_directories(${MY_INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/../gmp/include)
	IF (CMAKE_TencentSM_ARCH MATCHES "mac")
		MESSAGE(STATUS "current platform: Darwin")

		enable_language(C ASM)

		set (ASM_SOURCES
   		 ${PROJECT_SOURCE_DIR}/sm2/ecp_sm2z256-x86_64.s
		)

		set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/release/${CMAKE_TencentSM_ARCH}/${CMAKE_BUILD_TYPE})

		link_directories(${CMAKE_SOURCE_DIR}/gmp/lib/MacOS ${CMAKE_SOURCE_DIR}/libtasn1/product/lib ${MY_PROJECT_LINKDIR})

		add_library (${PROJECT_NAME} ${TencentSM_LIBTYPE} ${DIR_LIB_SRCS} ${ASM_SOURCES})
		IF (CMAKE_TencentSM_LIBTYPE MATCHES "static")
			target_link_libraries(${PROJECT_NAME} ${MY_ALLSUBLIBS} libgmp.a)
		ELSEIF (CMAKE_TencentSM_LIBTYPE MATCHES "shared")
			link_libraries(${CMAKE_SOURCE_DIR}/libtasn1/product/lib/libtasn1.a)
			link_libraries(${CMAKE_SOURCE_DIR}/gmp/lib/MacOS/libgmp.a)
			target_link_libraries(${PROJECT_NAME} -Wl,-all_load ${MY_ALLSUBLIBS} -Wl,-noall_load)
		ENDIF()
	ELSE (CMAKE_TencentSM_ARCH MATCHES "ios")
		MESSAGE(STATUS "current platform: ios")

		# IOS

		set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/release/ios/${IOS_PLATFORM})

		link_directories(${CMAKE_SOURCE_DIR}/gmp/lib/iOS/armv7s_arm64_x86_64 ${CMAKE_SOURCE_DIR}/libtasn1/product/lib ${MY_PROJECT_LINKDIR})

		IF (${SIMULATOR})
		  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mios-simulator-version-min=8.0")
		ENDIF()

		set(TencentSM_LIBTYPE STATIC)
		add_library (${PROJECT_NAME} STATIC ${DIR_LIB_SRCS})
		target_link_libraries(${PROJECT_NAME} libgmp.a libtasn1.a ${MY_ALLSUBLIBS})

		# Debug symbols set in XCode project
		set_xcode_property (${PROJECT_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS YES "All")

		# Installation
		#set (CMAKE_INSTALL_PREFIX "${example-ios_SOURCE_DIR}/../example-app/example-lib")
		#install (TARGETS example DESTINATION lib)
		#install (FILES ${HEADERS} DESTINATION include)
	ENDIF()

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Android")
	message(STATUS "Hello from Android build")
	include_directories(${MY_INCLUDE_PATH} ${PROJECT_SOURCE_DIR}/../gmp/include)
	# set(my_cxx_flags " -std=c99 -DOS_ANDROID -DJNI_INTERFACE -fPIC")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_C_FLAGS} ")
	add_definitions(-DOS_ANDROID)
	add_definitions(-DJNI_INTERFACE)
	# fPIC选项
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	#语言标准
	# set(CMAKE_C_STANDARD 11)
	# set(CMAKE_C_STANDARD_REQUIRED ON)
	IF (PARAM_ASN1_CODE_NO MATCHES "1")
		add_definitions(-DPARAM_ASN1_CODE_NO)
	ENDIF()
	set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/release/android/${ANDROID_ABI})
	IF (NOT(ANDROID_ABI MATCHES "arm64-v8a"))
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wall -v -Wl,--no-warn-shared-textrel")
	ENDIF()
	link_directories(${CMAKE_SOURCE_DIR}/gmp/lib/android/${ANDROID_ABI} ${CMAKE_SOURCE_DIR}/libtasn1/android/${ANDROID_ABI} ${MY_PROJECT_LINKDIR})

	# set(TencentSM_LIBTYPE SHARED)
	add_library (${PROJECT_NAME} ${TencentSM_LIBTYPE} ${DIR_LIB_SRCS})
    IF(CMAKE_BUILD_TYPE MATCHES "Debug")
        find_library( log-lib log)
    ENDIF()
	find_package(ZLIB)

	target_link_libraries(${PROJECT_NAME} ${log-lib} ${ZLIB_LIBRARIES} ${MY_ALLSUBLIBS} libgmp.a libtasn1.a)
ENDIF()

#subdir：不同平台的特性可以在本文件定义通过变量传递到子目录的makelists
foreach(subdir ${SUBDIR_NAMES})
	macro_add_subdir(${subdir})
endforeach(subdir)

#merge .a custom command
#cmake中多目录静态库无法链接成单一静态库，所以需要用shell命令实现

IF ( TencentSM_LIBTYPE MATCHES "STATIC" )
    SET(MY_OUTPUTPATH ${LIBRARY_OUTPUT_PATH}/lib${PROJECT_NAME}.a)
    message(STATUS "static lib need merge:${MY_ALLOBJS}" )
    message(STATUS "lib : ${MY_OUTPUTPATH}" )
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND
        ${CMAKE_AR} crs ${MY_OUTPUTPATH} ${MY_ALLOBJS}
        )
        
    IF (NOT CMAKE_TencentSM_ARCH MATCHES "ios")

    set(LIBTASN1_ALLOBJS ${LIBTASN1_ALLOBJS} ${CMAKE_SOURCE_DIR}/libtasn1/objs/*.o)

		message(STATUS "1:static lib need merge:${LIBTASN1_ALLOBJS}" )
		message(STATUS "lib : ${MY_OUTPUTPATH}" )
		add_custom_command(TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND
				${CMAKE_AR} crs ${MY_OUTPUTPATH} ${LIBTASN1_ALLOBJS}
				)

    IF ( ( EXISTS ${CMAKE_SOURCE_DIR}/gmp-6.1.2/objs ) AND NOT ( CMAKE_SYSTEM_NAME MATCHES "Windows" ) )
        set(LIBGMP_ALLOBJS ${LIBGMP_ALLOBJS} ${CMAKE_SOURCE_DIR}/gmp-6.1.2/objs/*.o)

        message(STATUS "2:static lib need merge:${LIBGMP_ALLOBJS}" )
        message(STATUS "lib : ${MY_OUTPUTPATH}" )
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND
            ${CMAKE_AR} crs ${MY_OUTPUTPATH} ${LIBGMP_ALLOBJS}
            )
    ENDIF()
    
    ENDIF()
ENDIF()


