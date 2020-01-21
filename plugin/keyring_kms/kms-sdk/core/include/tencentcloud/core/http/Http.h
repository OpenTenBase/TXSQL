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

#ifndef TENCENTCLOUD_CORE_HTTP_HTTPMESSAGE_H_
#define TENCENTCLOUD_CORE_HTTP_HTTPMESSAGE_H_

#include <map>
#include <string>

namespace TencentCloud
{
    class Http
    {
    private:
        struct NocaseLess
        {
            bool operator() (const std::string & s1, const std::string & s2) const;
        };
    public:
        typedef std::map<std::string, std::string, NocaseLess> HeaderCollection;

        Http();
        Http(const Http &other);
        Http(Http &&other);
        Http& operator=(const Http &other);
        Http& operator=(Http &&other);
        virtual ~Http();

        void AddHeader(const std::string &name, const std::string &value);
        const char* Body() const;
        size_t BodySize() const;
        bool HasBody() const;
        std::string Header(const std::string &name) const;
        HeaderCollection Headers() const;
        void RemoveHeader(const std::string &name);
        void SetBody(const std::string &body);
        void SetHeader(const std::string &name, const std::string &value);

    private:
        HeaderCollection m_headers;
        std::string m_body;
    };
}
#endif // !TENCENTCLOUD_CORE_HTTP_HTTPMESSAGE_H_
