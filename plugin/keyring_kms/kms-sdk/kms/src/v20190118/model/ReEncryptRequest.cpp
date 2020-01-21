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

#include <tencentcloud/kms/v20190118/model/ReEncryptRequest.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

ReEncryptRequest::ReEncryptRequest() :
    m_ciphertextBlobHasBeenSet(false),
    m_destinationKeyIdHasBeenSet(false),
    m_sourceEncryptionContextHasBeenSet(false),
    m_destinationEncryptionContextHasBeenSet(false)
{
}

string ReEncryptRequest::ToJsonString() const
{
    Document d;
    d.SetObject();
    Document::AllocatorType& allocator = d.GetAllocator();


    if (m_ciphertextBlobHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "CiphertextBlob";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_ciphertextBlob.c_str(), allocator).Move(), allocator);
    }

    if (m_destinationKeyIdHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "DestinationKeyId";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_destinationKeyId.c_str(), allocator).Move(), allocator);
    }

    if (m_sourceEncryptionContextHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "SourceEncryptionContext";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_sourceEncryptionContext.c_str(), allocator).Move(), allocator);
    }

    if (m_destinationEncryptionContextHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "DestinationEncryptionContext";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_destinationEncryptionContext.c_str(), allocator).Move(), allocator);
    }


    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}


string ReEncryptRequest::GetCiphertextBlob() const
{
    return m_ciphertextBlob;
}

void ReEncryptRequest::SetCiphertextBlob(const string& _ciphertextBlob)
{
    m_ciphertextBlob = _ciphertextBlob;
    m_ciphertextBlobHasBeenSet = true;
}

bool ReEncryptRequest::CiphertextBlobHasBeenSet() const
{
    return m_ciphertextBlobHasBeenSet;
}

string ReEncryptRequest::GetDestinationKeyId() const
{
    return m_destinationKeyId;
}

void ReEncryptRequest::SetDestinationKeyId(const string& _destinationKeyId)
{
    m_destinationKeyId = _destinationKeyId;
    m_destinationKeyIdHasBeenSet = true;
}

bool ReEncryptRequest::DestinationKeyIdHasBeenSet() const
{
    return m_destinationKeyIdHasBeenSet;
}

string ReEncryptRequest::GetSourceEncryptionContext() const
{
    return m_sourceEncryptionContext;
}

void ReEncryptRequest::SetSourceEncryptionContext(const string& _sourceEncryptionContext)
{
    m_sourceEncryptionContext = _sourceEncryptionContext;
    m_sourceEncryptionContextHasBeenSet = true;
}

bool ReEncryptRequest::SourceEncryptionContextHasBeenSet() const
{
    return m_sourceEncryptionContextHasBeenSet;
}

string ReEncryptRequest::GetDestinationEncryptionContext() const
{
    return m_destinationEncryptionContext;
}

void ReEncryptRequest::SetDestinationEncryptionContext(const string& _destinationEncryptionContext)
{
    m_destinationEncryptionContext = _destinationEncryptionContext;
    m_destinationEncryptionContextHasBeenSet = true;
}

bool ReEncryptRequest::DestinationEncryptionContextHasBeenSet() const
{
    return m_destinationEncryptionContextHasBeenSet;
}


