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

#include <tencentcloud/kms/v20190118/model/EncryptRequest.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

EncryptRequest::EncryptRequest() :
    m_keyIdHasBeenSet(false),
    m_plaintextHasBeenSet(false),
    m_encryptionContextHasBeenSet(false)
{
}

string EncryptRequest::ToJsonString() const
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

    if (m_plaintextHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Plaintext";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_plaintext.c_str(), allocator).Move(), allocator);
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


string EncryptRequest::GetKeyId() const
{
    return m_keyId;
}

void EncryptRequest::SetKeyId(const string& _keyId)
{
    m_keyId = _keyId;
    m_keyIdHasBeenSet = true;
}

bool EncryptRequest::KeyIdHasBeenSet() const
{
    return m_keyIdHasBeenSet;
}

string EncryptRequest::GetPlaintext() const
{
    return m_plaintext;
}

void EncryptRequest::SetPlaintext(const string& _plaintext)
{
    m_plaintext = _plaintext;
    m_plaintextHasBeenSet = true;
}

bool EncryptRequest::PlaintextHasBeenSet() const
{
    return m_plaintextHasBeenSet;
}

string EncryptRequest::GetEncryptionContext() const
{
    return m_encryptionContext;
}

void EncryptRequest::SetEncryptionContext(const string& _encryptionContext)
{
    m_encryptionContext = _encryptionContext;
    m_encryptionContextHasBeenSet = true;
}

bool EncryptRequest::EncryptionContextHasBeenSet() const
{
    return m_encryptionContextHasBeenSet;
}


