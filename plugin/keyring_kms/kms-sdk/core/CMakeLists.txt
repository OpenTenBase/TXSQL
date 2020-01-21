#
# Copyright (c) 2017-2019 THL A29 Limited, a Tencent company. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
configure_file(src/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/tencentcloud/core/Config.h @ONLY)

if(DEFINED RAPIDJSON_NO_SIZETYPEDEFINE)
remove_definitions(-DRAPIDJSON_NO_SIZETYPEDEFINE)
endif()

set(core_public_header
    include/tencentcloud/core/AbstractClient.h
    include/tencentcloud/core/AbstractModel.h
    include/tencentcloud/core/AsyncCallerContext.h
    include/tencentcloud/core/Config.h
    include/tencentcloud/core/Credential.h
    include/tencentcloud/core/Error.h
    include/tencentcloud/core/Executor.h
    include/tencentcloud/core/NetworkProxy.h
    include/tencentcloud/core/Outcome.h
    include/tencentcloud/core/Runnable.h
    include/tencentcloud/core/Sign.h
    include/tencentcloud/core/TencentCloud.h
    )

set(core_public_header_http
    include/tencentcloud/core/http/Http.h
    include/tencentcloud/core/http/HttpClient.h
    include/tencentcloud/core/http/HttpRequest.h
    include/tencentcloud/core/http/HttpResponse.h
    include/tencentcloud/core/http/Url.h
    )

set(core_public_header_profile
    include/tencentcloud/core/profile/ClientProfile.h
    include/tencentcloud/core/profile/HttpProfile.h
    )

set(core_public_header_utils
    include/tencentcloud/core/utils/Utils.h
    )

set(core_public_header_utils_rapidjson
    include/tencentcloud/core/utils/rapidjson
    )

set(core_src
    src/AbstractClient.cpp
    src/AbstractModel.cpp
    src/AsyncCallerContext.cpp
    src/Credential.cpp
    src/Executor.cpp
    src/NetworkProxy.cpp
    src/Sign.cpp
    src/TencentCloud.cpp
    src/http/Http.cpp
    src/http/HttpClient.cpp
    src/http/HttpRequest.cpp
    src/http/HttpResponse.cpp
    src/http/Url.cpp
    src/profile/ClientProfile.cpp
    src/profile/HttpProfile.cpp
    src/utils/Utils.cpp
    )

include_directories("include/tencentcloud/core/utils/rapidjson")
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CURL_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_library(core STATIC
    ${core_public_header}
    ${core_public_header_http}
    ${core_public_header_profile}
    ${core_public_header_utils}
    ${core_src})

set_target_properties(core
    PROPERTIES
    LINKER_LANGUAGE CXX
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}core
    )

target_include_directories(core
    PRIVATE include )

target_link_libraries(core
    uuid
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES} )
