SET(KEYRING_KMS_DIR_PATH ${CMAKE_SOURCE_DIR}/plugin/keyring_kms)
INCLUDE_DIRECTORIES(${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${KEYRING_KMS_DIR_PATH}/common)
INCLUDE_DIRECTORIES(${KEYRING_KMS_DIR_PATH}/kms-sdk/kms/include)
INCLUDE_DIRECTORIES(${KEYRING_KMS_DIR_PATH}/kms-sdk/sts/include)
INCLUDE_DIRECTORIES(${KEYRING_KMS_DIR_PATH}/kms-sdk/core/include)

if(NOT TARGET core)
  ADD_SUBDIRECTORY(${KEYRING_KMS_DIR_PATH}/kms-sdk/core core_dir)
endif()

if(NOT TARGET sts)
  ADD_SUBDIRECTORY(${KEYRING_KMS_DIR_PATH}/kms-sdk/sts sts_dir)
endif()

if(NOT TARGET kms)
  ADD_SUBDIRECTORY(${KEYRING_KMS_DIR_PATH}/kms-sdk/kms kms_dir)
endif()

ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="client_mysqlbinlog_keyring")

ADD_LIBRARY(mysqlbinlog_keyring STATIC
  mysqlbinlog_keyring.h
  mysqlbinlog_keyring.cc
  ${KEYRING_KMS_DIR_PATH}/common/jsoncpp.cpp
  ${KEYRING_KMS_DIR_PATH}/converter.cc
  ${KEYRING_KMS_DIR_PATH}/checker/checker.cc
  ${KEYRING_KMS_DIR_PATH}/checker/checker_factory.cc
  ${KEYRING_KMS_DIR_PATH}/checker/checker_ver_1_0.cc
  ${KEYRING_KMS_DIR_PATH}/checker/checker_ver_2_0.cc
  ${KEYRING_KMS_DIR_PATH}/digest.cc
  ${KEYRING_KMS_DIR_PATH}/common/keyring_impl.cc
  ${KEYRING_KMS_DIR_PATH}/common/keys_iterator.cc
)

TARGET_LINK_LIBRARIES(mysqlbinlog_keyring uuid core sts kms  ${CURL_LIBRARY} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY})
