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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_CREATEKEYREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_CREATEKEYREQUEST_H_

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
                * CreateKey请求参数结构体
                */
                class CreateKeyRequest : public AbstractModel
                {
                public:
                    CreateKeyRequest();
                    ~CreateKeyRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取作为密钥更容易辨识，更容易被人看懂的别名， 不可为空，1-60个字母数字 - _ 的组合。以 kms- 作为前缀的用于云产品使用，Alias 不可重复。
                     * @return Alias 作为密钥更容易辨识，更容易被人看懂的别名， 不可为空，1-60个字母数字 - _ 的组合。以 kms- 作为前缀的用于云产品使用，Alias 不可重复。
                     */
                    std::string GetAlias() const;

                    /**
                     * 设置作为密钥更容易辨识，更容易被人看懂的别名， 不可为空，1-60个字母数字 - _ 的组合。以 kms- 作为前缀的用于云产品使用，Alias 不可重复。
                     * @param Alias 作为密钥更容易辨识，更容易被人看懂的别名， 不可为空，1-60个字母数字 - _ 的组合。以 kms- 作为前缀的用于云产品使用，Alias 不可重复。
                     */
                    void SetAlias(const std::string& _alias);

                    /**
                     * 判断参数 Alias 是否已赋值
                     * @return Alias 是否已赋值
                     */
                    bool AliasHasBeenSet() const;

                    /**
                     * 获取CMK 的描述，最大1024字节
                     * @return Description CMK 的描述，最大1024字节
                     */
                    std::string GetDescription() const;

                    /**
                     * 设置CMK 的描述，最大1024字节
                     * @param Description CMK 的描述，最大1024字节
                     */
                    void SetDescription(const std::string& _description);

                    /**
                     * 判断参数 Description 是否已赋值
                     * @return Description 是否已赋值
                     */
                    bool DescriptionHasBeenSet() const;

                    /**
                     * 获取指定key的用途。目前，仅支持"ENCRYPT_DECRYPT"，默认为  "ENCRYPT_DECRYPT"，即key用于加密和解密
                     * @return KeyUsage 指定key的用途。目前，仅支持"ENCRYPT_DECRYPT"，默认为  "ENCRYPT_DECRYPT"，即key用于加密和解密
                     */
                    std::string GetKeyUsage() const;

                    /**
                     * 设置指定key的用途。目前，仅支持"ENCRYPT_DECRYPT"，默认为  "ENCRYPT_DECRYPT"，即key用于加密和解密
                     * @param KeyUsage 指定key的用途。目前，仅支持"ENCRYPT_DECRYPT"，默认为  "ENCRYPT_DECRYPT"，即key用于加密和解密
                     */
                    void SetKeyUsage(const std::string& _keyUsage);

                    /**
                     * 判断参数 KeyUsage 是否已赋值
                     * @return KeyUsage 是否已赋值
                     */
                    bool KeyUsageHasBeenSet() const;

                    /**
                     * 获取指定key类型，1为当前地域默认类型，默认为1，且当前只支持该类型
                     * @return Type 指定key类型，1为当前地域默认类型，默认为1，且当前只支持该类型
                     */
                    uint64_t GetType() const;

                    /**
                     * 设置指定key类型，1为当前地域默认类型，默认为1，且当前只支持该类型
                     * @param Type 指定key类型，1为当前地域默认类型，默认为1，且当前只支持该类型
                     */
                    void SetType(const uint64_t& _type);

                    /**
                     * 判断参数 Type 是否已赋值
                     * @return Type 是否已赋值
                     */
                    bool TypeHasBeenSet() const;

                private:

                    /**
                     * 作为密钥更容易辨识，更容易被人看懂的别名， 不可为空，1-60个字母数字 - _ 的组合。以 kms- 作为前缀的用于云产品使用，Alias 不可重复。
                     */
                    std::string m_alias;
                    bool m_aliasHasBeenSet;

                    /**
                     * CMK 的描述，最大1024字节
                     */
                    std::string m_description;
                    bool m_descriptionHasBeenSet;

                    /**
                     * 指定key的用途。目前，仅支持"ENCRYPT_DECRYPT"，默认为  "ENCRYPT_DECRYPT"，即key用于加密和解密
                     */
                    std::string m_keyUsage;
                    bool m_keyUsageHasBeenSet;

                    /**
                     * 指定key类型，1为当前地域默认类型，默认为1，且当前只支持该类型
                     */
                    uint64_t m_type;
                    bool m_typeHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_CREATEKEYREQUEST_H_
