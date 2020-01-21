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

#ifndef TENCENTCLOUD_STS_V20180813_MODEL_GETFEDERATIONTOKENREQUEST_H_
#define TENCENTCLOUD_STS_V20180813_MODEL_GETFEDERATIONTOKENREQUEST_H_

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
                * GetFederationToken请求参数结构体
                */
                class GetFederationTokenRequest : public AbstractModel
                {
                public:
                    GetFederationTokenRequest();
                    ~GetFederationTokenRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取联合身份用户昵称
                     * @return Name 联合身份用户昵称
                     */
                    std::string GetName() const;

                    /**
                     * 设置联合身份用户昵称
                     * @param Name 联合身份用户昵称
                     */
                    void SetName(const std::string& _name);

                    /**
                     * 判断参数 Name 是否已赋值
                     * @return Name 是否已赋值
                     */
                    bool NameHasBeenSet() const;

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

                    /**
                     * 获取指定临时证书的有效期，单位：秒，默认1800秒，最长可设定有效期为7200秒
                     * @return DurationSeconds 指定临时证书的有效期，单位：秒，默认1800秒，最长可设定有效期为7200秒
                     */
                    uint64_t GetDurationSeconds() const;

                    /**
                     * 设置指定临时证书的有效期，单位：秒，默认1800秒，最长可设定有效期为7200秒
                     * @param DurationSeconds 指定临时证书的有效期，单位：秒，默认1800秒，最长可设定有效期为7200秒
                     */
                    void SetDurationSeconds(const uint64_t& _durationSeconds);

                    /**
                     * 判断参数 DurationSeconds 是否已赋值
                     * @return DurationSeconds 是否已赋值
                     */
                    bool DurationSecondsHasBeenSet() const;

                private:

                    /**
                     * 联合身份用户昵称
                     */
                    std::string m_name;
                    bool m_nameHasBeenSet;

                    /**
                     * 策略描述
注意：
1、policy 需要做 urlencode（如果通过 GET 方法请求云 API，发送请求前，所有参数都需要按照云 API 规范再 urlencode 一次）。
2、策略语法参照 CAM 策略语法。
3、策略中不能包含 principal 元素。
                     */
                    std::string m_policy;
                    bool m_policyHasBeenSet;

                    /**
                     * 指定临时证书的有效期，单位：秒，默认1800秒，最长可设定有效期为7200秒
                     */
                    uint64_t m_durationSeconds;
                    bool m_durationSecondsHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_STS_V20180813_MODEL_GETFEDERATIONTOKENREQUEST_H_
