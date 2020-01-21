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

#include <tencentcloud/core/http/Url.h>
#include <sstream>

using namespace TencentCloud;
using namespace std;

namespace
{
#define INVALID_PORT -1
}

Url::Url() :
    m_scheme("https"),
    m_host(""),
    m_path("/"),
    m_query(""),
    m_port(INVALID_PORT)
{
}

void Url::SetHost(const string & host)
{
    m_host = host;
}

string Url::GetHost() const
{
    return m_host;
}

void Url::SetPath(const string & path)
{
    m_path = path;
}

string Url::GetPath() const
{
    return m_path;
}

bool Url::HasQuery() const
{
    return (!m_query.empty());
}

string Url::ToString() const
{
    std::ostringstream out;
    out << m_scheme << "://" << m_host;
    if (m_port!=INVALID_PORT)
        out << ":" << m_port;
    if (m_path.empty())
        out << "/";
    else
        out << m_path;
    if (HasQuery())
        out << "?" << m_query;
    return out.str();
}
