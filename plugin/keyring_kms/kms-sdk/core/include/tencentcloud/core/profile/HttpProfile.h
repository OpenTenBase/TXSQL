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

#ifndef TENCENTCLOUD_CORE_HTTPPROFILE_H_
#define TENCENTCLOUD_CORE_HTTPPROFILE_H_

#include <string>

namespace TencentCloud
{
    static const int64_t TM_MINUTE_SECOND = 60;

    class HttpProfile
    {
    public:
        enum Scheme
        {
            HTTPS,
        };
        enum Method
        {
            POST,
        };

        HttpProfile();
        void SetMethod(const Method &method);
        void SetEndpoint(const std::string &endpoint);
        std::string GetEndpoint() const;
        void SetProtocol(const HttpProfile::Scheme &protocol);
        void SetReqTimeout(int64_t timeoutOfSecond);
        int64_t GetReqTimeout() const;
        void SetConnectTimeout(int64_t timeoutOfSecond);
        int64_t GetConnectTimeout() const;
        void SetKeepAlive(bool flag=false);
        bool IsKeepAlive() const;

        HttpProfile(const HttpProfile &o) :
            m_reqMethod(o.m_reqMethod),
            m_endpoint(o.m_endpoint),
            m_protocol(o.m_protocol),
            m_reqTimeout(o.m_reqTimeout),
            m_connectTimeout(o.m_connectTimeout),
            m_keepAlive(o.m_keepAlive)
        {
        }

        HttpProfile & operator=(const HttpProfile &o)
        {
            if (this != &o)
            {
                m_reqMethod = o.m_reqMethod;
                m_endpoint = o.m_endpoint;
                m_protocol = o.m_protocol;
                m_reqTimeout = o.m_reqTimeout;
                m_connectTimeout = o.m_connectTimeout;
                m_keepAlive = o.m_keepAlive;
            }
            return *this;
        }
        HttpProfile & operator=(HttpProfile &&o)
        {
            if (this != &o)
            {
                *this = std::move(o);
            }
            return *this;
        }

    private:
        Method m_reqMethod;
        std::string m_endpoint;
        Scheme m_protocol;
        int64_t m_reqTimeout;
        int64_t m_connectTimeout;
        bool m_keepAlive;
    };
}

#endif // !TENCENTCLOUD_CORE_HTTPPROFILE_H_
