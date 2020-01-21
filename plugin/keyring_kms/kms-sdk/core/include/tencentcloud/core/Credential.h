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

#ifndef TENCENTCLOUD_CORE_CREDENTIAL_H_
#define TENCENTCLOUD_CORE_CREDENTIAL_H_

#include <string>

namespace TencentCloud
{
    class Credential
    {
    public:
        Credential(const std::string &secretId, const std::string &secretKey);
        Credential(const std::string &secretId, const std::string &secretKey, const std::string &token);
        void SetSecretId(const std::string &secretId);
        void SetSecretKey(const std::string &secertKey);
        void SetToken(const std::string &token);
        std::string GetSecretId() const;
        std::string GetSecretKey() const;
        std::string GetToken() const;

    private:
        std::string m_secretId;
        std::string m_secretKey;
        std::string m_token;
    };
}

#endif // !TENCENTCLOUD_CORE_CREDENTIAL_H_
