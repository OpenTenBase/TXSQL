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

#include <tencentcloud/core/http/HttpRequest.h>

using namespace TencentCloud;
using namespace std;

HttpRequest::HttpRequest(const Url &url, Method method) :
    Http(),
    m_url(url),
    m_method(method)
{
}

HttpRequest::Method HttpRequest::GetMethod() const
{
    return m_method;
}

void HttpRequest::SetMethod(Method method)
{
    m_method = method;
}

void HttpRequest::SetUrl(const Url &url)
{
    m_url = url;
}

const Url& HttpRequest::GetUrl() const
{
    return m_url;
}
