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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_UPDATEALIASREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_UPDATEALIASREQUEST_H_

#include <string>
#include <vector>
#include <map>
#include <tencentcloud/core/AbstractModel.h>


namespace TencentCloud
{
    namespace Kms
    {
        namespace V20190118
        {
            namespace Model
            {
                /**
                * UpdateAlias请求参数结构体
                */
                class UpdateAliasRequest : public AbstractModel
                {
                public:
                    UpdateAliasRequest();
                    ~UpdateAliasRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取新的别名，1-60个字符或数字的组合
                     * @return Alias 新的别名，1-60个字符或数字的组合
                     */
                    std::string GetAlias() const;

                    /**
                     * 设置新的别名，1-60个字符或数字的组合
                     * @param Alias 新的别名，1-60个字符或数字的组合
                     */
                    void SetAlias(const std::string& _alias);

                    /**
                     * 判断参数 Alias 是否已赋值
                     * @return Alias 是否已赋值
                     */
                    bool AliasHasBeenSet() const;

                    /**
                     * 获取CMK的全局唯一标识符
                     * @return KeyId CMK的全局唯一标识符
                     */
                    std::string GetKeyId() const;

                    /**
                     * 设置CMK的全局唯一标识符
                     * @param KeyId CMK的全局唯一标识符
                     */
                    void SetKeyId(const std::string& _keyId);

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                private:

                    /**
                     * 新的别名，1-60个字符或数字的组合
                     */
                    std::string m_alias;
                    bool m_aliasHasBeenSet;

                    /**
                     * CMK的全局唯一标识符
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_UPDATEALIASREQUEST_H_
