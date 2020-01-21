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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_REENCRYPTRESPONSE_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_REENCRYPTRESPONSE_H_

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
                * ReEncrypt返回参数结构体
                */
                class ReEncryptResponse : public AbstractModel
                {
                public:
                    ReEncryptResponse();
                    ~ReEncryptResponse() = default;
                    CoreInternalOutcome Deserialize(const std::string &payload);


                    /**
                     * 获取重新加密后的密文
                     * @return CiphertextBlob 重新加密后的密文
                     */
                    std::string GetCiphertextBlob() const;

                    /**
                     * 判断参数 CiphertextBlob 是否已赋值
                     * @return CiphertextBlob 是否已赋值
                     */
                    bool CiphertextBlobHasBeenSet() const;

                    /**
                     * 获取重新加密使用的CMK
                     * @return KeyId 重新加密使用的CMK
                     */
                    std::string GetKeyId() const;

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                    /**
                     * 获取重新加密前密文使用的CMK
                     * @return SourceKeyId 重新加密前密文使用的CMK
                     */
                    std::string GetSourceKeyId() const;

                    /**
                     * 判断参数 SourceKeyId 是否已赋值
                     * @return SourceKeyId 是否已赋值
                     */
                    bool SourceKeyIdHasBeenSet() const;

                    /**
                     * 获取true表示密文已经重新加密。同一个CMK进行重加密，在密钥没有发生轮换的情况下不会进行实际重新加密操作，返回原密文
                     * @return ReEncrypted true表示密文已经重新加密。同一个CMK进行重加密，在密钥没有发生轮换的情况下不会进行实际重新加密操作，返回原密文
                     */
                    bool GetReEncrypted() const;

                    /**
                     * 判断参数 ReEncrypted 是否已赋值
                     * @return ReEncrypted 是否已赋值
                     */
                    bool ReEncryptedHasBeenSet() const;

                private:

                    /**
                     * 重新加密后的密文
                     */
                    std::string m_ciphertextBlob;
                    bool m_ciphertextBlobHasBeenSet;

                    /**
                     * 重新加密使用的CMK
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                    /**
                     * 重新加密前密文使用的CMK
                     */
                    std::string m_sourceKeyId;
                    bool m_sourceKeyIdHasBeenSet;

                    /**
                     * true表示密文已经重新加密。同一个CMK进行重加密，在密钥没有发生轮换的情况下不会进行实际重新加密操作，返回原密文
                     */
                    bool m_reEncrypted;
                    bool m_reEncryptedHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_REENCRYPTRESPONSE_H_
