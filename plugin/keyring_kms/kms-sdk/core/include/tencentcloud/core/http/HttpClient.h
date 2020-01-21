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

#ifndef TENCENTCLOUD_CORE_HTTP_HTTPCLIENT_H_
#define TENCENTCLOUD_CORE_HTTP_HTTPCLIENT_H_

#include <curl/curl.h>
#include <tencentcloud/core/Outcome.h>
#include <tencentcloud/core/Error.h>
#include <tencentcloud/core/NetworkProxy.h>
#include "HttpResponse.h"

namespace TencentCloud
{
    class HttpClient
    {
    public:
        typedef Outcome<Error, HttpResponse> HttpResponseOutcome;

        HttpClient();
        ~HttpClient();

        void SetReqTimeout(int64_t timeoutOfMs);
        void SetConnectTimeout(int64_t timeoutOfMs);

        HttpResponseOutcome SendRequest(const HttpRequest &request);

        void SetProxy(const NetworkProxy &proxy);

        static void InitGlobalState();
        static void CleanupGlobalState();

    private:
        CURL *m_curlHandle;
        int64_t m_reqTimeout;
        int64_t m_connectTimeout;
        NetworkProxy m_proxy;
    };
}

#endif // !TENCENTCLOUD_CORE_HTTP_HTTPCLIENT_H_
