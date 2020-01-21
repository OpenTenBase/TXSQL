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

#include <tencentcloud/core/profile/HttpProfile.h>

using namespace TencentCloud;
using namespace std;

HttpProfile::HttpProfile() :
    m_reqMethod(HttpProfile::Method::POST),
    m_endpoint(""),
    m_protocol(HttpProfile::Scheme::HTTPS),
    m_reqTimeout(TM_MINUTE_SECOND),
    m_connectTimeout(TM_MINUTE_SECOND),
    m_keepAlive(false)
{
}

void HttpProfile::SetMethod(const HttpProfile::Method &method)
{
    m_reqMethod = method;
}

void HttpProfile::SetEndpoint(const string &endpoint)
{
    m_endpoint = endpoint;
}

string HttpProfile::GetEndpoint() const
{
    return m_endpoint;
}

void HttpProfile::SetProtocol(const HttpProfile::Scheme &protocol)
{
    m_protocol = protocol;
}

void HttpProfile::SetReqTimeout(int64_t timeoutOfSecond)
{
    m_reqTimeout = timeoutOfSecond;
}

int64_t HttpProfile::GetReqTimeout() const
{
    return m_reqTimeout;
}

void HttpProfile::SetConnectTimeout(int64_t timeoutOfSecond)
{
    m_connectTimeout = timeoutOfSecond;
}

int64_t HttpProfile::GetConnectTimeout() const
{
    return m_connectTimeout;
}

void HttpProfile::SetKeepAlive(bool flag)
{
    m_keepAlive = flag;
}

bool HttpProfile::IsKeepAlive() const
{
    return m_keepAlive;
}
