## Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

IF(WITH_ENTERPRISE_ENCRYPTION)
  SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
  INCLUDE(check_openssl)
  MYSQL_CHECK_OPENSSL()

  SET(CMAKE_CXX_FLAGES_DEBUG "${CMAKE_CXX_FLAGES} -O0 -pipe -Wall -fno-strict-aliasing  -fPIC -g -ggdb -lcrypto")
  SET(CMAKE_CXX_FLAGES_RELEASE"${CMAKE_CXX_FLAGES} -O1 -pipe -Wall -fno-strict-aliasing -fPIC -lcrypto")
  add_compile_options(-Wno-error=deprecated-declarations)


  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  MYSQL_ADD_PLUGIN(cdb_enterprise_encryption
                   cdb_enterprise_encryption.cc
                   cdb_enterprise_encryption_plugin.cc
                   MODULE_ONLY
                   MODULE_OUTPUT_NAME "cdb_enterprise_encryption")

  TARGET_LINK_LIBRARIES(cdb_enterprise_encryption  ${OPENSSL_LIBRARY} ${CRYPTO_LIBRARY})
ENDIF()
