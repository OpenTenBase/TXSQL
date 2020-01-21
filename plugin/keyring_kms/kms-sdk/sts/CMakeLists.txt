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

if(DEFINED RAPIDJSON_NO_SIZETYPEDEFINE)
  remove_definitions(-DRAPIDJSON_NO_SIZETYPEDEFINE)
endif()

set(sts_public_header
  include/tencentcloud/sts/v20180813/StsClient.h
  )

set(sts_public_header_model
  include/tencentcloud/sts/v20180813/model/AssumeRoleWithSAMLResponse.h
  include/tencentcloud/sts/v20180813/model/AssumeRoleWithSAMLRequest.h
  include/tencentcloud/sts/v20180813/model/GetFederationTokenResponse.h
  include/tencentcloud/sts/v20180813/model/AssumeRoleResponse.h
  include/tencentcloud/sts/v20180813/model/GetFederationTokenRequest.h
  include/tencentcloud/sts/v20180813/model/Credentials.h
  include/tencentcloud/sts/v20180813/model/AssumeRoleRequest.h
  )

set(sts_src
  src/v20180813/StsClient.cpp
  src/v20180813/model/AssumeRoleWithSAMLResponse.cpp
  src/v20180813/model/AssumeRoleWithSAMLRequest.cpp
  src/v20180813/model/GetFederationTokenResponse.cpp
  src/v20180813/model/AssumeRoleResponse.cpp
  src/v20180813/model/GetFederationTokenRequest.cpp
  src/v20180813/model/Credentials.cpp
  src/v20180813/model/AssumeRoleRequest.cpp
  )

set(sts_include
    include/tencentcloud/sts
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_library(sts STATIC 
  ${sts_public_header}
  ${sts_public_header_model}
  ${sts_src})

set_target_properties(sts
  PROPERTIES
  LINKER_LANGUAGE CXX
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}sts
  )

target_include_directories(sts
  PRIVATE include
    ${CMAKE_SOURCE_DIR}/plugin/keyring_kms/kms-sdk/core/include
  )
target_link_libraries(sts
  core)
