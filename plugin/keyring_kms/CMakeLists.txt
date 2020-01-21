# Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

IF(WITH_KMS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  ADD_SUBDIRECTORY(kms-sdk/kms)
  ADD_SUBDIRECTORY(kms-sdk/core)
  ADD_SUBDIRECTORY(kms-sdk/sts)

  ADD_DEFINITIONS(-DLOG_COMPONENT_TAG="keyring_kms")

  SET (KEYRING_KMS_SOURCES
       common/keyring_key.cc
       common/keys_container.cc
       common/keys_iterator.cc
       common/keyring_impl.cc
       common/jsoncpp.cpp
       checker/checker.cc
       checker/checker_factory.cc
       keyring.cc
       keyring_kms.cc
       hash_to_buffer_serializer.cc
       buffered_file_io.cc
       converter.cc
       buffer.cc
       digest.cc
       checker/checker_ver_1_0.cc
       checker/checker_ver_2_0.cc
       file_io.cc)

  # Build keyring kms
  INCLUDE_DIRECTORIES(${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/kms-sdk/kms/include)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/kms-sdk/sts/include)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/kms-sdk/core/include)

  MYSQL_ADD_PLUGIN(keyring_kms
                   ${KEYRING_KMS_SOURCES}
                   MODULE_ONLY
                   MODULE_OUTPUT_NAME "keyring_kms")

  # Static link
  TARGET_LINK_LIBRARIES(keyring_kms uuid core sts kms  ${CURL_LIBRARY}
                        ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY})

  IF(HAVE_DLOPEN)
    SET(DYNLIB_EXTENSION "so")
    SET_TARGET_PROPERTIES(keyring_kms
      PROPERTIES LINK_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
  ENDIF()

 IF(WITH_KMS_TEST)
    MYSQL_ADD_EXECUTABLE(kms-test kms_test.cc)
    TARGET_LINK_LIBRARIES(kms-test uuid core sts kms  ${CURL_LIBRARY} ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY})
  ENDIF()
ENDIF()
