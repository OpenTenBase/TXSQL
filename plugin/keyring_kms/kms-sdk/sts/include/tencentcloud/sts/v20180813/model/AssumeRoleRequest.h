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

#ifndef TENCENTCLOUD_STS_V20180813_MODEL_ASSUMEROLEREQUEST_H_
#define TENCENTCLOUD_STS_V20180813_MODEL_ASSUMEROLEREQUEST_H_

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
                * AssumeRole请求参数结构体
                */
                class AssumeRoleRequest : public AbstractModel
                {
                public:
                    AssumeRoleRequest();
                    ~AssumeRoleRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取角色的资源描述。例如：qcs::cam::uin/12345678:role/4611686018427397919、qcs::cam::uin/12345678:roleName/testRoleName
                     * @return RoleArn 角色的资源描述。例如：qcs::cam::uin/12345678:role/4611686018427397919、qcs::cam::uin/12345678:roleName/testRoleName
                     */
                    std::string GetRoleArn() const;

                    /**
                     * 设置角色的资源描述。例如：qcs::cam::uin/12345678:role/4611686018427397919、qcs::cam::uin/12345678:roleName/testRoleName
                     * @param RoleArn 角色的资源描述。例如：qcs::cam::uin/12345678:role/4611686018427397919、qcs::cam::uin/12345678:roleName/testRoleName
                     */
                    void SetRoleArn(const std::string& _roleArn);

                    /**
                     * 判断参数 RoleArn 是否已赋值
                     * @return RoleArn 是否已赋值
                     */
                    bool RoleArnHasBeenSet() const;

                    /**
                     * 获取临时会话名称，由用户自定义名称
                     * @return RoleSessionName 临时会话名称，由用户自定义名称
                     */
                    std::string GetRoleSessionName() const;

                    /**
                     * 设置临时会话名称，由用户自定义名称
                     * @param RoleSessionName 临时会话名称，由用户自定义名称
                     */
                    void SetRoleSessionName(const std::string& _roleSessionName);

                    /**
                     * 判断参数 RoleSessionName 是否已赋值
                     * @return RoleSessionName 是否已赋值
                     */
                    bool RoleSessionNameHasBeenSet() const;

                    /**
                     * 获取指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 43200 秒
                     * @return DurationSeconds 指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 43200 秒
                     */
                    uint64_t GetDurationSeconds() const;

                    /**
                     * 设置指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 43200 秒
                     * @param DurationSeconds 指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 43200 秒
                     */
                    void SetDurationSeconds(const uint64_t& _durationSeconds);

                    /**
                     * 判断参数 DurationSeconds 是否已赋值
                     * @return DurationSeconds 是否已赋值
                     */
                    bool DurationSecondsHasBeenSet() const;

                    /**
                     * 获取策略描述
注意：
1、policy 需要做 urlencode（如果通过 GET 方法请求云 API，发送请求前，所有参数都需要按照云 API 规范再 urlencode 一次）。
2、策略语法参照 CAM 策略语法。
3、策略中不能包含 principal 元素。
                     * @return Policy 策略描述
注意：
1、policy 需要做 urlencode（如果通过 GET 方法请求云 API，发送请求前，所有参数都需要按照云 API 规范再 urlencode 一次）。
2、策略语法参照 CAM 策略语法。
3、策略中不能包含 principal 元素。
                     */
                    std::string GetPolicy() const;

                    /**
                     * 设置策略描述
注意：
1、policy 需要做 urlencode（如果通过 GET 方法请求云 API，发送请求前，所有参数都需要按照云 API 规范再 urlencode 一次）。
2、策略语法参照 CAM 策略语法。
3、策略中不能包含 principal 元素。
                     * @param Policy 策略描述
注意：
1、policy 需要做 urlencode（如果通过 GET 方法请求云 API，发送请求前，所有参数都需要按照云 API 规范再 urlencode 一次）。
2、策略语法参照 CAM 策略语法。
3、策略中不能包含 principal 元素。
                     */
                    void SetPolicy(const std::string& _policy);

                    /**
                     * 判断参数 Policy 是否已赋值
                     * @return Policy 是否已赋值
                     */
                    bool PolicyHasBeenSet() const;

                private:

                    /**
                     * 角色的资源描述。例如：qcs::cam::uin/12345678:role/4611686018427397919、qcs::cam::uin/12345678:roleName/testRoleName
                     */
                    std::string m_roleArn;
                    bool m_roleArnHasBeenSet;

                    /**
                     * 临时会话名称，由用户自定义名称
                     */
                    std::string m_roleSessionName;
                    bool m_roleSessionNameHasBeenSet;

                    /**
                     * 指定临时证书的有效期，单位：秒，默认 7200 秒，最长可设定有效期为 43200 秒
                     */
                    uint64_t m_durationSeconds;
                    bool m_durationSecondsHasBeenSet;

                    /**
                     * 策略描述
注意：
1、policy 需要做 urlencode（如果通过 GET 方法请求云 API，发送请求前，所有参数都需要按照云 API 规范再 urlencode 一次）。
2、策略语法参照 CAM 策略语法。
3、策略中不能包含 principal 元素。
                     */
                    std::string m_policy;
                    bool m_policyHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_STS_V20180813_MODEL_ASSUMEROLEREQUEST_H_
