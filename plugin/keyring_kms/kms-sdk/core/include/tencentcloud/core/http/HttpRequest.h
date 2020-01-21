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

#ifndef TENCENTCLOUD_CORE_HTTP_HTTPREQUEST_H_
#define TENCENTCLOUD_CORE_HTTP_HTTPREQUEST_H_

#include "Http.h"
#include "Url.h"

namespace TencentCloud
{
    class HttpRequest : public Http
    {
    public:
        enum Method
        {
            POST
        };

        explicit HttpRequest(const Url &url = Url(), Method method = POST);
        ~HttpRequest() = default;

        Method GetMethod() const;
        void SetMethod(Method method);
        void SetUrl(const Url &url);
        const Url& GetUrl() const;

    private:
        Url m_url;
	Method m_method;
    };
}

#endif // !TENCENTCLOUD_CORE_HTTP_HTTPREQUEST_H_
