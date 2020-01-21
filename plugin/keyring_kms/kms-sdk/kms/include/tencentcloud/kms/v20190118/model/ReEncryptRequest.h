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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_REENCRYPTREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_REENCRYPTREQUEST_H_

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
                * ReEncrypt请求参数结构体
                */
                class ReEncryptRequest : public AbstractModel
                {
                public:
                    ReEncryptRequest();
                    ~ReEncryptRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取需要重新加密的密文
                     * @return CiphertextBlob 需要重新加密的密文
                     */
                    std::string GetCiphertextBlob() const;

                    /**
                     * 设置需要重新加密的密文
                     * @param CiphertextBlob 需要重新加密的密文
                     */
                    void SetCiphertextBlob(const std::string& _ciphertextBlob);

                    /**
                     * 判断参数 CiphertextBlob 是否已赋值
                     * @return CiphertextBlob 是否已赋值
                     */
                    bool CiphertextBlobHasBeenSet() const;

                    /**
                     * 获取重新加密使用的CMK，如果为空，则使用密文原有的CMK重新加密（若密钥没有轮换则密文不会刷新）
                     * @return DestinationKeyId 重新加密使用的CMK，如果为空，则使用密文原有的CMK重新加密（若密钥没有轮换则密文不会刷新）
                     */
                    std::string GetDestinationKeyId() const;

                    /**
                     * 设置重新加密使用的CMK，如果为空，则使用密文原有的CMK重新加密（若密钥没有轮换则密文不会刷新）
                     * @param DestinationKeyId 重新加密使用的CMK，如果为空，则使用密文原有的CMK重新加密（若密钥没有轮换则密文不会刷新）
                     */
                    void SetDestinationKeyId(const std::string& _destinationKeyId);

                    /**
                     * 判断参数 DestinationKeyId 是否已赋值
                     * @return DestinationKeyId 是否已赋值
                     */
                    bool DestinationKeyIdHasBeenSet() const;

                    /**
                     * 获取CiphertextBlob 密文加密时使用的key/value对的json字符串。如果加密时未使用，则为空
                     * @return SourceEncryptionContext CiphertextBlob 密文加密时使用的key/value对的json字符串。如果加密时未使用，则为空
                     */
                    std::string GetSourceEncryptionContext() const;

                    /**
                     * 设置CiphertextBlob 密文加密时使用的key/value对的json字符串。如果加密时未使用，则为空
                     * @param SourceEncryptionContext CiphertextBlob 密文加密时使用的key/value对的json字符串。如果加密时未使用，则为空
                     */
                    void SetSourceEncryptionContext(const std::string& _sourceEncryptionContext);

                    /**
                     * 判断参数 SourceEncryptionContext 是否已赋值
                     * @return SourceEncryptionContext 是否已赋值
                     */
                    bool SourceEncryptionContextHasBeenSet() const;

                    /**
                     * 获取重新加密使用的key/value对的json字符串，如果使用该字段，则返回的新密文在解密时需要填入相同的字符串
                     * @return DestinationEncryptionContext 重新加密使用的key/value对的json字符串，如果使用该字段，则返回的新密文在解密时需要填入相同的字符串
                     */
                    std::string GetDestinationEncryptionContext() const;

                    /**
                     * 设置重新加密使用的key/value对的json字符串，如果使用该字段，则返回的新密文在解密时需要填入相同的字符串
                     * @param DestinationEncryptionContext 重新加密使用的key/value对的json字符串，如果使用该字段，则返回的新密文在解密时需要填入相同的字符串
                     */
                    void SetDestinationEncryptionContext(const std::string& _destinationEncryptionContext);

                    /**
                     * 判断参数 DestinationEncryptionContext 是否已赋值
                     * @return DestinationEncryptionContext 是否已赋值
                     */
                    bool DestinationEncryptionContextHasBeenSet() const;

                private:

                    /**
                     * 需要重新加密的密文
                     */
                    std::string m_ciphertextBlob;
                    bool m_ciphertextBlobHasBeenSet;

                    /**
                     * 重新加密使用的CMK，如果为空，则使用密文原有的CMK重新加密（若密钥没有轮换则密文不会刷新）
                     */
                    std::string m_destinationKeyId;
                    bool m_destinationKeyIdHasBeenSet;

                    /**
                     * CiphertextBlob 密文加密时使用的key/value对的json字符串。如果加密时未使用，则为空
                     */
                    std::string m_sourceEncryptionContext;
                    bool m_sourceEncryptionContextHasBeenSet;

                    /**
                     * 重新加密使用的key/value对的json字符串，如果使用该字段，则返回的新密文在解密时需要填入相同的字符串
                     */
                    std::string m_destinationEncryptionContext;
                    bool m_destinationEncryptionContextHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_REENCRYPTREQUEST_H_
