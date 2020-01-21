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

#include <tencentcloud/sts/v20180813/model/AssumeRoleWithSAMLRequest.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using namespace TencentCloud::Sts::V20180813::Model;
using namespace rapidjson;
using namespace std;

AssumeRoleWithSAMLRequest::AssumeRoleWithSAMLRequest() :
    m_sAMLAssertionHasBeenSet(false),
    m_principalArnHasBeenSet(false),
    m_roleArnHasBeenSet(false),
    m_roleSessionNameHasBeenSet(false),
    m_durationSecondsHasBeenSet(false)
{
}

string AssumeRoleWithSAMLRequest::ToJsonString() const
{
    Document d;
    d.SetObject();
    Document::AllocatorType& allocator = d.GetAllocator();


    if (m_sAMLAssertionHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "SAMLAssertion";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_sAMLAssertion.c_str(), allocator).Move(), allocator);
    }

    if (m_principalArnHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "PrincipalArn";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_principalArn.c_str(), allocator).Move(), allocator);
    }

    if (m_roleArnHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "RoleArn";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_roleArn.c_str(), allocator).Move(), allocator);
    }

    if (m_roleSessionNameHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "RoleSessionName";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_roleSessionName.c_str(), allocator).Move(), allocator);
    }

    if (m_durationSecondsHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "DurationSeconds";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, m_durationSeconds, allocator);
    }


    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}


string AssumeRoleWithSAMLRequest::GetSAMLAssertion() const
{
    return m_sAMLAssertion;
}

void AssumeRoleWithSAMLRequest::SetSAMLAssertion(const string& _sAMLAssertion)
{
    m_sAMLAssertion = _sAMLAssertion;
    m_sAMLAssertionHasBeenSet = true;
}

bool AssumeRoleWithSAMLRequest::SAMLAssertionHasBeenSet() const
{
    return m_sAMLAssertionHasBeenSet;
}

string AssumeRoleWithSAMLRequest::GetPrincipalArn() const
{
    return m_principalArn;
}

void AssumeRoleWithSAMLRequest::SetPrincipalArn(const string& _principalArn)
{
    m_principalArn = _principalArn;
    m_principalArnHasBeenSet = true;
}

bool AssumeRoleWithSAMLRequest::PrincipalArnHasBeenSet() const
{
    return m_principalArnHasBeenSet;
}

string AssumeRoleWithSAMLRequest::GetRoleArn() const
{
    return m_roleArn;
}

void AssumeRoleWithSAMLRequest::SetRoleArn(const string& _roleArn)
{
    m_roleArn = _roleArn;
    m_roleArnHasBeenSet = true;
}

bool AssumeRoleWithSAMLRequest::RoleArnHasBeenSet() const
{
    return m_roleArnHasBeenSet;
}

string AssumeRoleWithSAMLRequest::GetRoleSessionName() const
{
    return m_roleSessionName;
}

void AssumeRoleWithSAMLRequest::SetRoleSessionName(const string& _roleSessionName)
{
    m_roleSessionName = _roleSessionName;
    m_roleSessionNameHasBeenSet = true;
}

bool AssumeRoleWithSAMLRequest::RoleSessionNameHasBeenSet() const
{
    return m_roleSessionNameHasBeenSet;
}

uint64_t AssumeRoleWithSAMLRequest::GetDurationSeconds() const
{
    return m_durationSeconds;
}

void AssumeRoleWithSAMLRequest::SetDurationSeconds(const uint64_t& _durationSeconds)
{
    m_durationSeconds = _durationSeconds;
    m_durationSecondsHasBeenSet = true;
}

bool AssumeRoleWithSAMLRequest::DurationSecondsHasBeenSet() const
{
    return m_durationSecondsHasBeenSet;
}


