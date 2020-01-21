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

#include <tencentcloud/sts/v20180813/model/GetFederationTokenRequest.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using namespace TencentCloud::Sts::V20180813::Model;
using namespace rapidjson;
using namespace std;

GetFederationTokenRequest::GetFederationTokenRequest() :
    m_nameHasBeenSet(false),
    m_policyHasBeenSet(false),
    m_durationSecondsHasBeenSet(false)
{
}

string GetFederationTokenRequest::ToJsonString() const
{
    Document d;
    d.SetObject();
    Document::AllocatorType& allocator = d.GetAllocator();


    if (m_nameHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Name";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_name.c_str(), allocator).Move(), allocator);
    }

    if (m_policyHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Policy";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, Value(m_policy.c_str(), allocator).Move(), allocator);
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


string GetFederationTokenRequest::GetName() const
{
    return m_name;
}

void GetFederationTokenRequest::SetName(const string& _name)
{
    m_name = _name;
    m_nameHasBeenSet = true;
}

bool GetFederationTokenRequest::NameHasBeenSet() const
{
    return m_nameHasBeenSet;
}

string GetFederationTokenRequest::GetPolicy() const
{
    return m_policy;
}

void GetFederationTokenRequest::SetPolicy(const string& _policy)
{
    m_policy = _policy;
    m_policyHasBeenSet = true;
}

bool GetFederationTokenRequest::PolicyHasBeenSet() const
{
    return m_policyHasBeenSet;
}

uint64_t GetFederationTokenRequest::GetDurationSeconds() const
{
    return m_durationSeconds;
}

void GetFederationTokenRequest::SetDurationSeconds(const uint64_t& _durationSeconds)
{
    m_durationSeconds = _durationSeconds;
    m_durationSecondsHasBeenSet = true;
}

bool GetFederationTokenRequest::DurationSecondsHasBeenSet() const
{
    return m_durationSecondsHasBeenSet;
}


