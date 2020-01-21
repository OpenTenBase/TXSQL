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

set(kms_public_header
  include/tencentcloud/kms/v20190118/KmsClient.h
  )

set(kms_public_header_model
  include/tencentcloud/kms/v20190118/model/DisableKeysRequest.h
  include/tencentcloud/kms/v20190118/model/UpdateAliasRequest.h
  include/tencentcloud/kms/v20190118/model/ReEncryptRequest.h
  include/tencentcloud/kms/v20190118/model/GenerateDataKeyRequest.h
  include/tencentcloud/kms/v20190118/model/GenerateDataKeyResponse.h
  include/tencentcloud/kms/v20190118/model/EnableKeyRotationRequest.h
  include/tencentcloud/kms/v20190118/model/EnableKeyRotationResponse.h
  include/tencentcloud/kms/v20190118/model/CreateKeyRequest.h
  include/tencentcloud/kms/v20190118/model/EnableKeysResponse.h
  include/tencentcloud/kms/v20190118/model/DisableKeyResponse.h
  include/tencentcloud/kms/v20190118/model/DecryptResponse.h
  include/tencentcloud/kms/v20190118/model/ReEncryptResponse.h
  include/tencentcloud/kms/v20190118/model/DescribeKeyRequest.h
  include/tencentcloud/kms/v20190118/model/CancelKeyDeletionRequest.h
  include/tencentcloud/kms/v20190118/model/EnableKeyResponse.h
  include/tencentcloud/kms/v20190118/model/ScheduleKeyDeletionRequest.h
  include/tencentcloud/kms/v20190118/model/GetServiceStatusResponse.h
  include/tencentcloud/kms/v20190118/model/Key.h
  include/tencentcloud/kms/v20190118/model/GetKeyRotationStatusResponse.h
  include/tencentcloud/kms/v20190118/model/EnableKeysRequest.h
  include/tencentcloud/kms/v20190118/model/UpdateKeyDescriptionResponse.h
  include/tencentcloud/kms/v20190118/model/EncryptRequest.h
  include/tencentcloud/kms/v20190118/model/CreateKeyResponse.h
  include/tencentcloud/kms/v20190118/model/EnableKeyRequest.h
  include/tencentcloud/kms/v20190118/model/GetKeyRotationStatusRequest.h
  include/tencentcloud/kms/v20190118/model/DescribeKeysResponse.h
  include/tencentcloud/kms/v20190118/model/ListKeyDetailResponse.h
  include/tencentcloud/kms/v20190118/model/CancelKeyDeletionResponse.h
  include/tencentcloud/kms/v20190118/model/ListKeysRequest.h
  include/tencentcloud/kms/v20190118/model/ListKeysResponse.h
  include/tencentcloud/kms/v20190118/model/KeyMetadata.h
  include/tencentcloud/kms/v20190118/model/DisableKeysResponse.h
  include/tencentcloud/kms/v20190118/model/DisableKeyRotationResponse.h
  include/tencentcloud/kms/v20190118/model/UpdateAliasResponse.h
  include/tencentcloud/kms/v20190118/model/ScheduleKeyDeletionResponse.h
  include/tencentcloud/kms/v20190118/model/DecryptRequest.h
  include/tencentcloud/kms/v20190118/model/ListKeyDetailRequest.h
  include/tencentcloud/kms/v20190118/model/UpdateKeyDescriptionRequest.h
  include/tencentcloud/kms/v20190118/model/DisableKeyRequest.h
  include/tencentcloud/kms/v20190118/model/DescribeKeyResponse.h
  include/tencentcloud/kms/v20190118/model/EncryptResponse.h
  include/tencentcloud/kms/v20190118/model/DescribeKeysRequest.h
  include/tencentcloud/kms/v20190118/model/GetServiceStatusRequest.h
  include/tencentcloud/kms/v20190118/model/DisableKeyRotationRequest.h
  )

set(kms_src
  src/v20190118/KmsClient.cpp
  src/v20190118/model/DisableKeysRequest.cpp
  src/v20190118/model/UpdateAliasRequest.cpp
  src/v20190118/model/ReEncryptRequest.cpp
  src/v20190118/model/GenerateDataKeyRequest.cpp
  src/v20190118/model/GenerateDataKeyResponse.cpp
  src/v20190118/model/EnableKeyRotationRequest.cpp
  src/v20190118/model/EnableKeyRotationResponse.cpp
  src/v20190118/model/CreateKeyRequest.cpp
  src/v20190118/model/EnableKeysResponse.cpp
  src/v20190118/model/DisableKeyResponse.cpp
  src/v20190118/model/DecryptResponse.cpp
  src/v20190118/model/ReEncryptResponse.cpp
  src/v20190118/model/DescribeKeyRequest.cpp
  src/v20190118/model/CancelKeyDeletionRequest.cpp
  src/v20190118/model/EnableKeyResponse.cpp
  src/v20190118/model/ScheduleKeyDeletionRequest.cpp
  src/v20190118/model/GetServiceStatusResponse.cpp
  src/v20190118/model/Key.cpp
  src/v20190118/model/GetKeyRotationStatusResponse.cpp
  src/v20190118/model/EnableKeysRequest.cpp
  src/v20190118/model/UpdateKeyDescriptionResponse.cpp
  src/v20190118/model/EncryptRequest.cpp
  src/v20190118/model/CreateKeyResponse.cpp
  src/v20190118/model/EnableKeyRequest.cpp
  src/v20190118/model/GetKeyRotationStatusRequest.cpp
  src/v20190118/model/DescribeKeysResponse.cpp
  src/v20190118/model/ListKeyDetailResponse.cpp
  src/v20190118/model/CancelKeyDeletionResponse.cpp
  src/v20190118/model/ListKeysRequest.cpp
  src/v20190118/model/ListKeysResponse.cpp
  src/v20190118/model/KeyMetadata.cpp
  src/v20190118/model/DisableKeysResponse.cpp
  src/v20190118/model/DisableKeyRotationResponse.cpp
  src/v20190118/model/UpdateAliasResponse.cpp
  src/v20190118/model/ScheduleKeyDeletionResponse.cpp
  src/v20190118/model/DecryptRequest.cpp
  src/v20190118/model/ListKeyDetailRequest.cpp
  src/v20190118/model/UpdateKeyDescriptionRequest.cpp
  src/v20190118/model/DisableKeyRequest.cpp
  src/v20190118/model/DescribeKeyResponse.cpp
  src/v20190118/model/EncryptResponse.cpp
  src/v20190118/model/DescribeKeysRequest.cpp
  src/v20190118/model/GetServiceStatusRequest.cpp
  src/v20190118/model/DisableKeyRotationRequest.cpp
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

set(kms_include
    include/tencentcloud/kms
  )
add_library(kms STATIC
  ${kms_public_header}
  ${kms_public_header_model}
  ${kms_src})

set_target_properties(kms
  PROPERTIES
  LINKER_LANGUAGE CXX
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  OUTPUT_NAME ${TARGET_OUTPUT_NAME_PREFIX}kms
  )
target_include_directories(kms
  PRIVATE include
    ${CMAKE_SOURCE_DIR}/plugin/keyring_kms/kms-sdk/core/include
  )
target_link_libraries(kms
  core)
