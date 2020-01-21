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

#include <tencentcloud/kms/v20190118/model/ReEncryptResponse.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using TencentCloud::CoreInternalOutcome;
using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

ReEncryptResponse::ReEncryptResponse() :
    m_ciphertextBlobHasBeenSet(false),
    m_keyIdHasBeenSet(false),
    m_sourceKeyIdHasBeenSet(false),
    m_reEncryptedHasBeenSet(false)
{
}

CoreInternalOutcome ReEncryptResponse::Deserialize(const string &payload)
{
    Document d;
    d.Parse(payload.c_str());
    if (d.HasParseError() || !d.IsObject())
    {
        return CoreInternalOutcome(Error("response not json format"));
    }
    if (!d.HasMember("Response") || !d["Response"].IsObject())
    {
        return CoreInternalOutcome(Error("response `Response` is null or not object"));
    }
    Value &rsp = d["Response"];
    if (!rsp.HasMember("RequestId") || !rsp["RequestId"].IsString())
    {
        return CoreInternalOutcome(Error("response `Response.RequestId` is null or not string"));
    }
    string requestId(rsp["RequestId"].GetString());
    SetRequestId(requestId);

    if (rsp.HasMember("Error"))
    {
        if (!rsp["Error"].IsObject() ||
            !rsp["Error"].HasMember("Code") || !rsp["Error"]["Code"].IsString() ||
            !rsp["Error"].HasMember("Message") || !rsp["Error"]["Message"].IsString())
        {
            return CoreInternalOutcome(Error("response `Response.Error` format error").SetRequestId(requestId));
        }
        string errorCode(rsp["Error"]["Code"].GetString());
        string errorMsg(rsp["Error"]["Message"].GetString());
        return CoreInternalOutcome(Error(errorCode, errorMsg).SetRequestId(requestId));
    }


    if (rsp.HasMember("CiphertextBlob") && !rsp["CiphertextBlob"].IsNull())
    {
        if (!rsp["CiphertextBlob"].IsString())
        {
            return CoreInternalOutcome(Error("response `CiphertextBlob` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_ciphertextBlob = string(rsp["CiphertextBlob"].GetString());
        m_ciphertextBlobHasBeenSet = true;
    }

    if (rsp.HasMember("KeyId") && !rsp["KeyId"].IsNull())
    {
        if (!rsp["KeyId"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyId` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyId = string(rsp["KeyId"].GetString());
        m_keyIdHasBeenSet = true;
    }

    if (rsp.HasMember("SourceKeyId") && !rsp["SourceKeyId"].IsNull())
    {
        if (!rsp["SourceKeyId"].IsString())
        {
            return CoreInternalOutcome(Error("response `SourceKeyId` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_sourceKeyId = string(rsp["SourceKeyId"].GetString());
        m_sourceKeyIdHasBeenSet = true;
    }

    if (rsp.HasMember("ReEncrypted") && !rsp["ReEncrypted"].IsNull())
    {
        if (!rsp["ReEncrypted"].IsBool())
        {
            return CoreInternalOutcome(Error("response `ReEncrypted` IsBool=false incorrectly").SetRequestId(requestId));
        }
        m_reEncrypted = rsp["ReEncrypted"].GetBool();
        m_reEncryptedHasBeenSet = true;
    }


    return CoreInternalOutcome(true);
}


string ReEncryptResponse::GetCiphertextBlob() const
{
    return m_ciphertextBlob;
}

bool ReEncryptResponse::CiphertextBlobHasBeenSet() const
{
    return m_ciphertextBlobHasBeenSet;
}

string ReEncryptResponse::GetKeyId() const
{
    return m_keyId;
}

bool ReEncryptResponse::KeyIdHasBeenSet() const
{
    return m_keyIdHasBeenSet;
}

string ReEncryptResponse::GetSourceKeyId() const
{
    return m_sourceKeyId;
}

bool ReEncryptResponse::SourceKeyIdHasBeenSet() const
{
    return m_sourceKeyIdHasBeenSet;
}

bool ReEncryptResponse::GetReEncrypted() const
{
    return m_reEncrypted;
}

bool ReEncryptResponse::ReEncryptedHasBeenSet() const
{
    return m_reEncryptedHasBeenSet;
}


