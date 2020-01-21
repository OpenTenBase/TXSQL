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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_DECRYPTREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_DECRYPTREQUEST_H_

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
                * Decrypt请求参数结构体
                */
                class DecryptRequest : public AbstractModel
                {
                public:
                    DecryptRequest();
                    ~DecryptRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取被加密的密文数据
                     * @return CiphertextBlob 被加密的密文数据
                     */
                    std::string GetCiphertextBlob() const;

                    /**
                     * 设置被加密的密文数据
                     * @param CiphertextBlob 被加密的密文数据
                     */
                    void SetCiphertextBlob(const std::string& _ciphertextBlob);

                    /**
                     * 判断参数 CiphertextBlob 是否已赋值
                     * @return CiphertextBlob 是否已赋值
                     */
                    bool CiphertextBlobHasBeenSet() const;

                    /**
                     * 获取key/value对的json字符串，如果Encrypt指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024字符
                     * @return EncryptionContext key/value对的json字符串，如果Encrypt指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024字符
                     */
                    std::string GetEncryptionContext() const;

                    /**
                     * 设置key/value对的json字符串，如果Encrypt指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024字符
                     * @param EncryptionContext key/value对的json字符串，如果Encrypt指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024字符
                     */
                    void SetEncryptionContext(const std::string& _encryptionContext);

                    /**
                     * 判断参数 EncryptionContext 是否已赋值
                     * @return EncryptionContext 是否已赋值
                     */
                    bool EncryptionContextHasBeenSet() const;

                private:

                    /**
                     * 被加密的密文数据
                     */
                    std::string m_ciphertextBlob;
                    bool m_ciphertextBlobHasBeenSet;

                    /**
                     * key/value对的json字符串，如果Encrypt指定了该参数，则在调用Decrypt API时需要提供同样的参数，最大支持1024字符
                     */
                    std::string m_encryptionContext;
                    bool m_encryptionContextHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_DECRYPTREQUEST_H_
