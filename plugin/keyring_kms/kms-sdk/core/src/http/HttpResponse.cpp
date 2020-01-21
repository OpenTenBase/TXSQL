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

#include <tencentcloud/core/http/HttpResponse.h>

namespace
{
    #define INVALID_STATUS_CODE -1
}

using namespace TencentCloud;

HttpResponse::HttpResponse() :
    m_statusCode(INVALID_STATUS_CODE)
{
}

void HttpResponse::SetStatusCode(int code)
{
    m_statusCode = code;
}

int HttpResponse::GetStatusCode() const
{
    return m_statusCode;
}
