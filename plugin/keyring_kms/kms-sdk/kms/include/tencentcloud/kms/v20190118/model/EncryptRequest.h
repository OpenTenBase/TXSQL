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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_ENCRYPTREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_ENCRYPTREQUEST_H_

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
                * Encrypt请求参数结构体
                */
                class EncryptRequest : public AbstractModel
                {
                public:
                    EncryptRequest();
                    ~EncryptRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取调用CreateKey生成的CMK全局唯一标识符
                     * @return KeyId 调用CreateKey生成的CMK全局唯一标识符
                     */
                    std::string GetKeyId() const;

                    /**
                     * 设置调用CreateKey生成的CMK全局唯一标识符
                     * @param KeyId 调用CreateKey生成的CMK全局唯一标识符
                     */
                    void SetKeyId(const std::string& _keyId);

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                    /**
                     * 获取被加密的明文数据，该字段必须使用base64编码，原文最大长度支持4K
                     * @return Plaintext 被加密的明文数据，该字段必须使用base64编码，原文最大长度支持4K
                     */
                    std::string GetPlaintext() const;

                    /**
                     * 设置被加密的明文数据，该字段必须使用base64编码，原文最大长度支持4K
                     * @param Plaintext 被加密的明文数据，该字段必须使用base64编码，原文最大长度支持4K
                     */
                    void SetPlaintext(const std::string& _plaintext);

                    /**
                     * 判断参数 Plaintext 是否已赋值
                     * @return Plaintext 是否已赋值
                     */
                    bool PlaintextHasBeenSet() const;

                    /**
                     * 获取key/value对的json字符串，如果指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024个字符
                     * @return EncryptionContext key/value对的json字符串，如果指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024个字符
                     */
                    std::string GetEncryptionContext() const;

                    /**
                     * 设置key/value对的json字符串，如果指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024个字符
                     * @param EncryptionContext key/value对的json字符串，如果指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024个字符
                     */
                    void SetEncryptionContext(const std::string& _encryptionContext);

                    /**
                     * 判断参数 EncryptionContext 是否已赋值
                     * @return EncryptionContext 是否已赋值
                     */
                    bool EncryptionContextHasBeenSet() const;

                private:

                    /**
                     * 调用CreateKey生成的CMK全局唯一标识符
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                    /**
                     * 被加密的明文数据，该字段必须使用base64编码，原文最大长度支持4K
                     */
                    std::string m_plaintext;
                    bool m_plaintextHasBeenSet;

                    /**
                     * key/value对的json字符串，如果指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024个字符
                     */
                    std::string m_encryptionContext;
                    bool m_encryptionContextHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_ENCRYPTREQUEST_H_
