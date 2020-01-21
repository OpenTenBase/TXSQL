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

#include <tencentcloud/sts/v20180813/model/Credentials.h>

using TencentCloud::CoreInternalOutcome;
using namespace TencentCloud::Sts::V20180813::Model;
using namespace rapidjson;
using namespace std;

Credentials::Credentials() :
    m_tokenHasBeenSet(false),
    m_tmpSecretIdHasBeenSet(false),
    m_tmpSecretKeyHasBeenSet(false)
{
}

CoreInternalOutcome Credentials::Deserialize(const Value &value)
{
    string requestId = "";


    if (value.HasMember("Token") && !value["Token"].IsNull())
    {
        if (!value["Token"].IsString())
        {
            return CoreInternalOutcome(Error("response `Credentials.Token` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_token = string(value["Token"].GetString());
        m_tokenHasBeenSet = true;
    }

    if (value.HasMember("TmpSecretId") && !value["TmpSecretId"].IsNull())
    {
        if (!value["TmpSecretId"].IsString())
        {
            return CoreInternalOutcome(Error("response `Credentials.TmpSecretId` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_tmpSecretId = string(value["TmpSecretId"].GetString());
        m_tmpSecretIdHasBeenSet = true;
    }

    if (value.HasMember("TmpSecretKey") && !value["TmpSecretKey"].IsNull())
    {
        if (!value["TmpSecretKey"].IsString())
        {
            return CoreInternalOutcome(Error("response `Credentials.TmpSecretKey` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_tmpSecretKey = string(value["TmpSecretKey"].GetString());
        m_tmpSecretKeyHasBeenSet = true;
    }


    return CoreInternalOutcome(true);
}

void Credentials::ToJsonObject(Value &value, Document::AllocatorType& allocator) const
{

    if (m_tokenHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Token";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_token.c_str(), allocator).Move(), allocator);
    }

    if (m_tmpSecretIdHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "TmpSecretId";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_tmpSecretId.c_str(), allocator).Move(), allocator);
    }

    if (m_tmpSecretKeyHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "TmpSecretKey";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_tmpSecretKey.c_str(), allocator).Move(), allocator);
    }

}


string Credentials::GetToken() const
{
    return m_token;
}

void Credentials::SetToken(const string& _token)
{
    m_token = _token;
    m_tokenHasBeenSet = true;
}

bool Credentials::TokenHasBeenSet() const
{
    return m_tokenHasBeenSet;
}

string Credentials::GetTmpSecretId() const
{
    return m_tmpSecretId;
}

void Credentials::SetTmpSecretId(const string& _tmpSecretId)
{
    m_tmpSecretId = _tmpSecretId;
    m_tmpSecretIdHasBeenSet = true;
}

bool Credentials::TmpSecretIdHasBeenSet() const
{
    return m_tmpSecretIdHasBeenSet;
}

string Credentials::GetTmpSecretKey() const
{
    return m_tmpSecretKey;
}

void Credentials::SetTmpSecretKey(const string& _tmpSecretKey)
{
    m_tmpSecretKey = _tmpSecretKey;
    m_tmpSecretKeyHasBeenSet = true;
}

bool Credentials::TmpSecretKeyHasBeenSet() const
{
    return m_tmpSecretKeyHasBeenSet;
}

