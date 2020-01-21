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

#ifndef TENCENTCLOUD_STS_V20180813_MODEL_ASSUMEROLEWITHSAMLREQUEST_H_
#define TENCENTCLOUD_STS_V20180813_MODEL_ASSUMEROLEWITHSAMLREQUEST_H_

#include <string>
#include <vector>
#include <map>
#include <tencentcloud/core/AbstractModel.h>


namespace TencentCloud
{
    namespace Sts
    {
        namespace V20180813
        {
            namespace Model
            {
                /**
                * AssumeRoleWithSAML请求参数结构体
                */
                class AssumeRoleWithSAMLRequest : public AbstractModel
                {
                public:
                    AssumeRoleWithSAMLRequest();
                    ~AssumeRoleWithSAMLRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取base64 编码的 SAML 断言信息
                     * @return SAMLAssertion base64 编码的 SAML 断言信息
                     */
                    std::string GetSAMLAssertion() const;

                    /**
                     * 设置base64 编码的 SAML 断言信息
                     * @param SAMLAssertion base64 编码的 SAML 断言信息
                     */
                    void SetSAMLAssertion(const std::string& _sAMLAssertion);

                    /**
                     * 判断参数 SAMLAssertion 是否已赋值
                     * @return SAMLAssertion 是否已赋值
                     */
                    bool SAMLAssertionHasBeenSet() const;

                    /**
                     * 获取扮演者访问描述名
                     * @return PrincipalArn 扮演者访问描述名
                     */
                    std::string GetPrincipalArn() const;

                    /**
                     * 设置扮演者访问描述名
                     * @param PrincipalArn 扮演者访问描述名
                     */
                    void SetPrincipalArn(const std::string& _principalArn);

                    /**
                     * 判断参数 PrincipalArn 是否已赋值
                     * @return PrincipalArn 是否已赋值
                     */
                    bool PrincipalArnHasBeenSet() const;

                    /**
                     * 获取角色访问描述名
                     * @return RoleArn 角色访问描述名
                     */
                    std::string GetRoleArn() const;

                    /**
                     * 设置角色访问描述名
                     * @param RoleArn 角色访问描述名
                     */
                    void SetRoleArn(const std::string& _roleArn);

                    /**
                     * 判断参数 RoleArn 是否已赋值
                     * @return RoleArn 是否已赋值
                     */
                    bool RoleArnHasBeenSet() const;

                    /**
                     * 获取会话名称
                     * @return RoleSessionName 会话名称
                     */
                    std::string GetRoleSessionName() const;

                    /**
                     * 设置会话名称
                     * @param RoleSessionName 会话名称
                     */
                    void SetRoleSessionName(const std::string& _roleSessionName);

                    /**
                     * 判断参数 RoleSessionName 是否已赋值
                     * @return RoleSessionName 是否已赋值
                     */
                    bool RoleSessionNameHasBeenSet() const;

                    /**
                     * 获取指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 7200 秒
                     * @return DurationSeconds 指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 7200 秒
                     */
                    uint64_t GetDurationSeconds() const;

                    /**
                     * 设置指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 7200 秒
                     * @param DurationSeconds 指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 7200 秒
                     */
                    void SetDurationSeconds(const uint64_t& _durationSeconds);

                    /**
                     * 判断参数 DurationSeconds 是否已赋值
                     * @return DurationSeconds 是否已赋值
                     */
                    bool DurationSecondsHasBeenSet() const;

                private:

                    /**
                     * base64 编码的 SAML 断言信息
                     */
                    std::string m_sAMLAssertion;
                    bool m_sAMLAssertionHasBeenSet;

                    /**
                     * 扮演者访问描述名
                     */
                    std::string m_principalArn;
                    bool m_principalArnHasBeenSet;

                    /**
                     * 角色访问描述名
                     */
                    std::string m_roleArn;
                    bool m_roleArnHasBeenSet;

                    /**
                     * 会话名称
                     */
                    std::string m_roleSessionName;
                    bool m_roleSessionNameHasBeenSet;

                    /**
                     * 指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 7200 秒
                     */
                    uint64_t m_durationSeconds;
                    bool m_durationSecondsHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_STS_V20180813_MODEL_ASSUMEROLEWITHSAMLREQUEST_H_
