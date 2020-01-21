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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_CREATEKEYRESPONSE_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_CREATEKEYRESPONSE_H_

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
                * CreateKey返回参数结构体
                */
                class CreateKeyResponse : public AbstractModel
                {
                public:
                    CreateKeyResponse();
                    ~CreateKeyResponse() = default;
                    CoreInternalOutcome Deserialize(const std::string &payload);


                    /**
                     * 获取CMK的全局唯一标识符
                     * @return KeyId CMK的全局唯一标识符
                     */
                    std::string GetKeyId() const;

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                    /**
                     * 获取作为密钥更容易辨识，更容易被人看懂的别名
                     * @return Alias 作为密钥更容易辨识，更容易被人看懂的别名
                     */
                    std::string GetAlias() const;

                    /**
                     * 判断参数 Alias 是否已赋值
                     * @return Alias 是否已赋值
                     */
                    bool AliasHasBeenSet() const;

                    /**
                     * 获取密钥创建时间，unix时间戳
                     * @return CreateTime 密钥创建时间，unix时间戳
                     */
                    uint64_t GetCreateTime() const;

                    /**
                     * 判断参数 CreateTime 是否已赋值
                     * @return CreateTime 是否已赋值
                     */
                    bool CreateTimeHasBeenSet() const;

                    /**
                     * 获取CMK的描述
注意：此字段可能返回 null，表示取不到有效值。
                     * @return Description CMK的描述
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    std::string GetDescription() const;

                    /**
                     * 判断参数 Description 是否已赋值
                     * @return Description 是否已赋值
                     */
                    bool DescriptionHasBeenSet() const;

                    /**
                     * 获取CMK的状态
                     * @return KeyState CMK的状态
                     */
                    std::string GetKeyState() const;

                    /**
                     * 判断参数 KeyState 是否已赋值
                     * @return KeyState 是否已赋值
                     */
                    bool KeyStateHasBeenSet() const;

                    /**
                     * 获取CMK的用途
                     * @return KeyUsage CMK的用途
                     */
                    std::string GetKeyUsage() const;

                    /**
                     * 判断参数 KeyUsage 是否已赋值
                     * @return KeyUsage 是否已赋值
                     */
                    bool KeyUsageHasBeenSet() const;

                private:

                    /**
                     * CMK的全局唯一标识符
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                    /**
                     * 作为密钥更容易辨识，更容易被人看懂的别名
                     */
                    std::string m_alias;
                    bool m_aliasHasBeenSet;

                    /**
                     * 密钥创建时间，unix时间戳
                     */
                    uint64_t m_createTime;
                    bool m_createTimeHasBeenSet;

                    /**
                     * CMK的描述
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    std::string m_description;
                    bool m_descriptionHasBeenSet;

                    /**
                     * CMK的状态
                     */
                    std::string m_keyState;
                    bool m_keyStateHasBeenSet;

                    /**
                     * CMK的用途
                     */
                    std::string m_keyUsage;
                    bool m_keyUsageHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_CREATEKEYRESPONSE_H_
