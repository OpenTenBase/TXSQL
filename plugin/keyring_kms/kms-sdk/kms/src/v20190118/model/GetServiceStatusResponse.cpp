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

#include <tencentcloud/kms/v20190118/model/GetServiceStatusResponse.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using TencentCloud::CoreInternalOutcome;
using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

GetServiceStatusResponse::GetServiceStatusResponse() :
    m_serviceEnabledHasBeenSet(false),
    m_invalidTypeHasBeenSet(false)
{
}

CoreInternalOutcome GetServiceStatusResponse::Deserialize(const string &payload)
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


    if (rsp.HasMember("ServiceEnabled") && !rsp["ServiceEnabled"].IsNull())
    {
        if (!rsp["ServiceEnabled"].IsBool())
        {
            return CoreInternalOutcome(Error("response `ServiceEnabled` IsBool=false incorrectly").SetRequestId(requestId));
        }
        m_serviceEnabled = rsp["ServiceEnabled"].GetBool();
        m_serviceEnabledHasBeenSet = true;
    }

    if (rsp.HasMember("InvalidType") && !rsp["InvalidType"].IsNull())
    {
        if (!rsp["InvalidType"].IsInt64())
        {
            return CoreInternalOutcome(Error("response `InvalidType` IsInt64=false incorrectly").SetRequestId(requestId));
        }
        m_invalidType = rsp["InvalidType"].GetInt64();
        m_invalidTypeHasBeenSet = true;
    }


    return CoreInternalOutcome(true);
}


bool GetServiceStatusResponse::GetServiceEnabled() const
{
    return m_serviceEnabled;
}

bool GetServiceStatusResponse::ServiceEnabledHasBeenSet() const
{
    return m_serviceEnabledHasBeenSet;
}

int64_t GetServiceStatusResponse::GetInvalidType() const
{
    return m_invalidType;
}

bool GetServiceStatusResponse::InvalidTypeHasBeenSet() const
{
    return m_invalidTypeHasBeenSet;
}


