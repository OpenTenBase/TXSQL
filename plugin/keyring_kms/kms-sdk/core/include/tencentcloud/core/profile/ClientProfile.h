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

#ifndef TENCENTCLOUD_CORE_CLIENTPROFILE_H_
#define TENCENTCLOUD_CORE_CLIENTPROFILE_H_

#include "HttpProfile.h"

namespace TencentCloud
{

    class ClientProfile
    {
    public:
        enum SignMethod
        {
            TC3_HMAC_SHA256,
        };

        ClientProfile() :
            ClientProfile(SignMethod::TC3_HMAC_SHA256)
        {
        }

        ClientProfile(const SignMethod &signMethod) :
            ClientProfile(signMethod, HttpProfile())
        {
        }

        ClientProfile(const SignMethod &signMethod, const HttpProfile &httpProfile) :
            m_httpProfile(httpProfile),
            m_unsignedPayload(false),
            m_signMethod(signMethod)
        {
        }

        ClientProfile(const HttpProfile &httpProfile) :
            ClientProfile(SignMethod::TC3_HMAC_SHA256, httpProfile)
        {
        }

        void SetSignMethod(const SignMethod &signMethod);
        void SetHttpProfile(const HttpProfile &httpProfile);
        HttpProfile GetHttpProfile();

    protected:
        void SetUnsignedPayload(bool flag);
        bool IsUnsignedPayload();

    private:
        HttpProfile m_httpProfile;
        bool m_unsignedPayload;
        SignMethod m_signMethod;
    };
}

#endif // !TENCENTCLOUD_CORE_CLIENTPROFILE_H_
