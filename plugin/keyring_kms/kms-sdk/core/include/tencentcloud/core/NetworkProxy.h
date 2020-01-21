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

#ifndef TENCENTCLOUD_CORE_NETWORKPROXY_H_
#define TENCENTCLOUD_CORE_NETWORKPROXY_H_

#include <string>

namespace TencentCloud
{
    class NetworkProxy
    {
        public:
            enum Type
            {
                NONE = 0,
                HTTP,
                SOCKS5
            };
            NetworkProxy(Type type = NONE,
                const std::string &hostName = "",
                uint16_t port = 0,
                const std::string &user = "",
                const std::string &password = "");

            ~NetworkProxy();

            std::string GetHostName() const;
            std::string GetPassword() const;
            uint16_t GetPort() const;
            void SetHostName(const std::string &hostName);
            void SetPassword(const std::string &password);
            void SetPort(uint16_t port);
            void SetType(Type type);
            void SetUser(const std::string &user);
            Type GetType() const;
            std::string GetUser() const;

        private:
            std::string m_hostName;
            std::string m_password;
            uint16_t m_port;
            Type m_type;
            std::string m_user;
    };
}
#endif // !TENCENTCLOUD_CORE_NETWORKPROXY_H_
