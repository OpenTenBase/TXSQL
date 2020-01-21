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

#include <tencentcloud/core/Credential.h>

using namespace TencentCloud;

Credential::Credential(const std::string &secretId, const std::string &secretKey) :
    Credential(secretId, secretKey, "")
{
}

Credential::Credential(const std::string &secretId, const std::string &secretKey, const std::string &token) :
    m_secretId(secretId),
    m_secretKey(secretKey),
    m_token(token)
{
}

void Credential::SetSecretId(const std::string &secretId)
{
    m_secretId = secretId;
}

void Credential::SetSecretKey(const std::string &secretKey)
{
    m_secretKey = secretKey;
}

void Credential::SetToken(const std::string &token)
{
    m_token = token;
}

std::string Credential::GetSecretId() const
{
    return m_secretId;
}

std::string Credential::GetSecretKey() const
{
    return m_secretKey;
}

std::string Credential::GetToken() const
{
    return m_token;
}
