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

#include <tencentcloud/sts/v20180813/model/AssumeRoleWithSAMLResponse.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using TencentCloud::CoreInternalOutcome;
using namespace TencentCloud::Sts::V20180813::Model;
using namespace rapidjson;
using namespace std;

AssumeRoleWithSAMLResponse::AssumeRoleWithSAMLResponse() :
    m_credentialsHasBeenSet(false),
    m_expiredTimeHasBeenSet(false),
    m_expirationHasBeenSet(false)
{
}

CoreInternalOutcome AssumeRoleWithSAMLResponse::Deserialize(const string &payload)
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


    if (rsp.HasMember("Credentials") && !rsp["Credentials"].IsNull())
    {
        if (!rsp["Credentials"].IsObject())
        {
            return CoreInternalOutcome(Error("response `Credentials` is not object type").SetRequestId(requestId));
        }

        CoreInternalOutcome outcome = m_credentials.Deserialize(rsp["Credentials"]);
        if (!outcome.IsSuccess())
        {
            outcome.GetError().SetRequestId(requestId);
            return outcome;
        }

        m_credentialsHasBeenSet = true;
    }

    if (rsp.HasMember("ExpiredTime") && !rsp["ExpiredTime"].IsNull())
    {
        if (!rsp["ExpiredTime"].IsUint64())
        {
            return CoreInternalOutcome(Error("response `ExpiredTime` IsUint64=false incorrectly").SetRequestId(requestId));
        }
        m_expiredTime = rsp["ExpiredTime"].GetUint64();
        m_expiredTimeHasBeenSet = true;
    }

    if (rsp.HasMember("Expiration") && !rsp["Expiration"].IsNull())
    {
        if (!rsp["Expiration"].IsString())
        {
            return CoreInternalOutcome(Error("response `Expiration` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_expiration = string(rsp["Expiration"].GetString());
        m_expirationHasBeenSet = true;
    }


    return CoreInternalOutcome(true);
}


Credentials AssumeRoleWithSAMLResponse::GetCredentials() const
{
    return m_credentials;
}

bool AssumeRoleWithSAMLResponse::CredentialsHasBeenSet() const
{
    return m_credentialsHasBeenSet;
}

uint64_t AssumeRoleWithSAMLResponse::GetExpiredTime() const
{
    return m_expiredTime;
}

bool AssumeRoleWithSAMLResponse::ExpiredTimeHasBeenSet() const
{
    return m_expiredTimeHasBeenSet;
}

string AssumeRoleWithSAMLResponse::GetExpiration() const
{
    return m_expiration;
}

bool AssumeRoleWithSAMLResponse::ExpirationHasBeenSet() const
{
    return m_expirationHasBeenSet;
}


