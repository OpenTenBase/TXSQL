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

#ifndef TENCENTCLOUD_STS_V20180813_STSCLIENT_H_
#define TENCENTCLOUD_STS_V20180813_STSCLIENT_H_

#include <functional>
#include <future>
#include <tencentcloud/core/AbstractClient.h>
#include <tencentcloud/core/Credential.h>
#include <tencentcloud/core/profile/ClientProfile.h>
#include <tencentcloud/core/AsyncCallerContext.h>
#include <tencentcloud/sts/v20180813/model/AssumeRoleRequest.h>
#include <tencentcloud/sts/v20180813/model/AssumeRoleResponse.h>
#include <tencentcloud/sts/v20180813/model/AssumeRoleWithSAMLRequest.h>
#include <tencentcloud/sts/v20180813/model/AssumeRoleWithSAMLResponse.h>
#include <tencentcloud/sts/v20180813/model/GetFederationTokenRequest.h>
#include <tencentcloud/sts/v20180813/model/GetFederationTokenResponse.h>


namespace TencentCloud
{
    namespace Sts
    {
        namespace V20180813
        {
            class StsClient : public AbstractClient
            {
            public:
                StsClient(const Credential &credential, const std::string &region);
                StsClient(const Credential &credential, const std::string &region, const ClientProfile &profile);

                typedef Outcome<Error, Model::AssumeRoleResponse> AssumeRoleOutcome;
                typedef std::future<AssumeRoleOutcome> AssumeRoleOutcomeCallable;
                typedef std::function<void(const StsClient*, const Model::AssumeRoleRequest&, AssumeRoleOutcome, const std::shared_ptr<const AsyncCallerContext>&)> AssumeRoleAsyncHandler;
                typedef Outcome<Error, Model::AssumeRoleWithSAMLResponse> AssumeRoleWithSAMLOutcome;
                typedef std::future<AssumeRoleWithSAMLOutcome> AssumeRoleWithSAMLOutcomeCallable;
                typedef std::function<void(const StsClient*, const Model::AssumeRoleWithSAMLRequest&, AssumeRoleWithSAMLOutcome, const std::shared_ptr<const AsyncCallerContext>&)> AssumeRoleWithSAMLAsyncHandler;
                typedef Outcome<Error, Model::GetFederationTokenResponse> GetFederationTokenOutcome;
                typedef std::future<GetFederationTokenOutcome> GetFederationTokenOutcomeCallable;
                typedef std::function<void(const StsClient*, const Model::GetFederationTokenRequest&, GetFederationTokenOutcome, const std::shared_ptr<const AsyncCallerContext>&)> GetFederationTokenAsyncHandler;



                /**
                 *申请扮演角色
                 * @param req AssumeRoleRequest
                 * @return AssumeRoleOutcome
                 */
                AssumeRoleOutcome AssumeRole(const Model::AssumeRoleRequest &request);
                void AssumeRoleAsync(const Model::AssumeRoleRequest& request, const AssumeRoleAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                AssumeRoleOutcomeCallable AssumeRoleCallable(const Model::AssumeRoleRequest& request);

                /**
                 *本接口（AssumeRoleWithSAML）用于根据 SAML 断言申请角色临时凭证。
                 * @param req AssumeRoleWithSAMLRequest
                 * @return AssumeRoleWithSAMLOutcome
                 */
                AssumeRoleWithSAMLOutcome AssumeRoleWithSAML(const Model::AssumeRoleWithSAMLRequest &request);
                void AssumeRoleWithSAMLAsync(const Model::AssumeRoleWithSAMLRequest& request, const AssumeRoleWithSAMLAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                AssumeRoleWithSAMLOutcomeCallable AssumeRoleWithSAMLCallable(const Model::AssumeRoleWithSAMLRequest& request);

                /**
                 *获取联合身份临时访问凭证
                 * @param req GetFederationTokenRequest
                 * @return GetFederationTokenOutcome
                 */
                GetFederationTokenOutcome GetFederationToken(const Model::GetFederationTokenRequest &request);
                void GetFederationTokenAsync(const Model::GetFederationTokenRequest& request, const GetFederationTokenAsyncHandler& handler, const std::shared_ptr<const AsyncCallerContext>& context = nullptr);
                GetFederationTokenOutcomeCallable GetFederationTokenCallable(const Model::GetFederationTokenRequest& request);

            };
        }
    }
}

#endif // !TENCENTCLOUD_STS_V20180813_STSCLIENT_H_
