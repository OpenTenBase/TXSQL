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

#include <tencentcloud/kms/v20190118/model/ListKeysRequest.h>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>

using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

ListKeysRequest::ListKeysRequest() :
    m_offsetHasBeenSet(false),
    m_limitHasBeenSet(false),
    m_roleHasBeenSet(false)
{
}

string ListKeysRequest::ToJsonString() const
{
    Document d;
    d.SetObject();
    Document::AllocatorType& allocator = d.GetAllocator();


    if (m_offsetHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Offset";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, m_offset, allocator);
    }

    if (m_limitHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Limit";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, m_limit, allocator);
    }

    if (m_roleHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Role";
        iKey.SetString(key.c_str(), allocator);
        d.AddMember(iKey, m_role, allocator);
    }


    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}


uint64_t ListKeysRequest::GetOffset() const
{
    return m_offset;
}

void ListKeysRequest::SetOffset(const uint64_t& _offset)
{
    m_offset = _offset;
    m_offsetHasBeenSet = true;
}

bool ListKeysRequest::OffsetHasBeenSet() const
{
    return m_offsetHasBeenSet;
}

uint64_t ListKeysRequest::GetLimit() const
{
    return m_limit;
}

void ListKeysRequest::SetLimit(const uint64_t& _limit)
{
    m_limit = _limit;
    m_limitHasBeenSet = true;
}

bool ListKeysRequest::LimitHasBeenSet() const
{
    return m_limitHasBeenSet;
}

uint64_t ListKeysRequest::GetRole() const
{
    return m_role;
}

void ListKeysRequest::SetRole(const uint64_t& _role)
{
    m_role = _role;
    m_roleHasBeenSet = true;
}

bool ListKeysRequest::RoleHasBeenSet() const
{
    return m_roleHasBeenSet;
}


