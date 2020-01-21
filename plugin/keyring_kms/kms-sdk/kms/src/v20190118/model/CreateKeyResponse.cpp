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

#include <tencentcloud/kms/v20190118/model/CreateKeyResponse.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using TencentCloud::CoreInternalOutcome;
using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

CreateKeyResponse::CreateKeyResponse() :
    m_keyIdHasBeenSet(false),
    m_aliasHasBeenSet(false),
    m_createTimeHasBeenSet(false),
    m_descriptionHasBeenSet(false),
    m_keyStateHasBeenSet(false),
    m_keyUsageHasBeenSet(false)
{
}

CoreInternalOutcome CreateKeyResponse::Deserialize(const string &payload)
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


    if (rsp.HasMember("KeyId") && !rsp["KeyId"].IsNull())
    {
        if (!rsp["KeyId"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyId` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyId = string(rsp["KeyId"].GetString());
        m_keyIdHasBeenSet = true;
    }

    if (rsp.HasMember("Alias") && !rsp["Alias"].IsNull())
    {
        if (!rsp["Alias"].IsString())
        {
            return CoreInternalOutcome(Error("response `Alias` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_alias = string(rsp["Alias"].GetString());
        m_aliasHasBeenSet = true;
    }

    if (rsp.HasMember("CreateTime") && !rsp["CreateTime"].IsNull())
    {
        if (!rsp["CreateTime"].IsUint64())
        {
            return CoreInternalOutcome(Error("response `CreateTime` IsUint64=false incorrectly").SetRequestId(requestId));
        }
        m_createTime = rsp["CreateTime"].GetUint64();
        m_createTimeHasBeenSet = true;
    }

    if (rsp.HasMember("Description") && !rsp["Description"].IsNull())
    {
        if (!rsp["Description"].IsString())
        {
            return CoreInternalOutcome(Error("response `Description` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_description = string(rsp["Description"].GetString());
        m_descriptionHasBeenSet = true;
    }

    if (rsp.HasMember("KeyState") && !rsp["KeyState"].IsNull())
    {
        if (!rsp["KeyState"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyState` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyState = string(rsp["KeyState"].GetString());
        m_keyStateHasBeenSet = true;
    }

    if (rsp.HasMember("KeyUsage") && !rsp["KeyUsage"].IsNull())
    {
        if (!rsp["KeyUsage"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyUsage` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyUsage = string(rsp["KeyUsage"].GetString());
        m_keyUsageHasBeenSet = true;
    }


    return CoreInternalOutcome(true);
}


string CreateKeyResponse::GetKeyId() const
{
    return m_keyId;
}

bool CreateKeyResponse::KeyIdHasBeenSet() const
{
    return m_keyIdHasBeenSet;
}

string CreateKeyResponse::GetAlias() const
{
    return m_alias;
}

bool CreateKeyResponse::AliasHasBeenSet() const
{
    return m_aliasHasBeenSet;
}

uint64_t CreateKeyResponse::GetCreateTime() const
{
    return m_createTime;
}

bool CreateKeyResponse::CreateTimeHasBeenSet() const
{
    return m_createTimeHasBeenSet;
}

string CreateKeyResponse::GetDescription() const
{
    return m_description;
}

bool CreateKeyResponse::DescriptionHasBeenSet() const
{
    return m_descriptionHasBeenSet;
}

string CreateKeyResponse::GetKeyState() const
{
    return m_keyState;
}

bool CreateKeyResponse::KeyStateHasBeenSet() const
{
    return m_keyStateHasBeenSet;
}

string CreateKeyResponse::GetKeyUsage() const
{
    return m_keyUsage;
}

bool CreateKeyResponse::KeyUsageHasBeenSet() const
{
    return m_keyUsageHasBeenSet;
}


