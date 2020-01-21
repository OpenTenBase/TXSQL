/*
 * Copyright (c) 2017-2019 THL A29 Limited, a Tencent company. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <tencentcloud/kms/v20190118/model/GenerateDataKeyRequest.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

GenerateDataKeyRequest::GenerateDataKeyRequest() :
    m_keyIdHasBeenSet(false),
    m_keySpecHasBeenSet(false),
    m_numberOfBytesHasBeenSet(false),
    m_encryptionContextHasBeenSet(false)
{
}

string GenerateDataKeyRequest::ToJsonString() const
{
    Document d;
    d.SetObject();
    Document::AllocatorType& allocator = d.GetAllocator();


    if (m_keyIdHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "KeyId";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_keyId.c_str(), allocator).Move(), allocator);
    }

    if (m_keySpecHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "KeySpec";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_keySpec.c_str(), allocator).Move(), allocator);
    }

    if (m_numberOfBytesHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "NumberOfBytes";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, m_numberOfBytes, allocator);
    }

    if (m_encryptionContextHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "EncryptionContext";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_encryptionContext.c_str(), allocator).Move(), allocator);
    }


    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}


string GenerateDataKeyRequest::GetKeyId() const
{
    return m_keyId;
}

void GenerateDataKeyRequest::SetKeyId(const string& _keyId)
{
    m_keyId = _keyId;
    m_keyIdHasBeenSet = true;
}

bool GenerateDataKeyRequest::KeyIdHasBeenSet() const
{
    return m_keyIdHasBeenSet;
}

string GenerateDataKeyRequest::GetKeySpec() const
{
    return m_keySpec;
}

void GenerateDataKeyRequest::SetKeySpec(const string& _keySpec)
{
    m_keySpec = _keySpec;
    m_keySpecHasBeenSet = true;
}

bool GenerateDataKeyRequest::KeySpecHasBeenSet() const
{
    return m_keySpecHasBeenSet;
}

uint64_t GenerateDataKeyRequest::GetNumberOfBytes() const
{
    return m_numberOfBytes;
}

void GenerateDataKeyRequest::SetNumberOfBytes(const uint64_t& _numberOfBytes)
{
    m_numberOfBytes = _numberOfBytes;
    m_numberOfBytesHasBeenSet = true;
}

bool GenerateDataKeyRequest::NumberOfBytesHasBeenSet() const
{
    return m_numberOfBytesHasBeenSet;
}

string GenerateDataKeyRequest::GetEncryptionContext() const
{
    return m_encryptionContext;
}

void GenerateDataKeyRequest::SetEncryptionContext(const string& _encryptionContext)
{
    m_encryptionContext = _encryptionContext;
    m_encryptionContextHasBeenSet = true;
}

bool GenerateDataKeyRequest::EncryptionContextHasBeenSet() const
{
    return m_encryptionContextHasBeenSet;
}


