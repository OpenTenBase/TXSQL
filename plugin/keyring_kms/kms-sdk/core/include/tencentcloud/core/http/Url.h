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

#ifndef TENCENTCLOUD_CORE_HTTP_URL_H_
#define TENCENTCLOUD_CORE_HTTP_URL_H_

#include <string>

namespace TencentCloud
{
    class Url
    {
    public:
        Url();
        ~Url() = default;

        void SetHost(const std::string &host);
        std::string GetHost() const;
        void SetPath(const std::string &path);
        std::string GetPath() const;
        bool HasQuery() const;
        std::string ToString() const;

    public:
        int m_iURLType;

        std::string m_scheme;
        std::string m_host;
        std::string m_path;
        std::string m_query;
        int m_port;
    };
}

#endif // !TENCENTCLOUD_CORE_HTTP_URL_H_
