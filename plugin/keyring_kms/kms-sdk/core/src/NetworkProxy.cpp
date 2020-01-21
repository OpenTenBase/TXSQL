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

#include <tencentcloud/core/NetworkProxy.h>

using namespace TencentCloud;

NetworkProxy::NetworkProxy(Type type,
        const std::string &hostName,
        uint16_t port,
        const std::string &user,
        const std::string &password) :
    m_hostName(hostName),
    m_password(password),
    m_port(port),
    m_type(type),
    m_user(user)
{
}

NetworkProxy::~NetworkProxy()
{
}

std::string NetworkProxy::GetHostName() const
{
    return m_hostName;
}

std::string NetworkProxy::GetPassword() const
{
    return m_password;
}

uint16_t NetworkProxy::GetPort() const
{
    return m_port;
}

void NetworkProxy::SetHostName(const std::string &hostName)
{
    m_hostName = hostName;
}

void NetworkProxy::SetPassword(const std::string &password)
{
    m_password = password;
}

void NetworkProxy::SetPort(uint16_t port)
{
    m_port = port;
}

void NetworkProxy::SetType(NetworkProxy::Type type)
{
    m_type = type;
}

void NetworkProxy::SetUser(const std::string &user)
{
    m_user = user;
}

NetworkProxy::Type NetworkProxy::GetType() const
{
    return m_type;
}

std::string NetworkProxy::GetUser() const
{
    return m_user;
}
