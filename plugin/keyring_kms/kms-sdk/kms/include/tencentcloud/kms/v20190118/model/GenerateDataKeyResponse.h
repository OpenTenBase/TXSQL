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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_GENERATEDATAKEYRESPONSE_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_GENERATEDATAKEYRESPONSE_H_

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
                * GenerateDataKey返回参数结构体
                */
                class GenerateDataKeyResponse : public AbstractModel
                {
                public:
                    GenerateDataKeyResponse();
                    ~GenerateDataKeyResponse() = default;
                    CoreInternalOutcome Deserialize(const std::string &payload);


                    /**
                     * 获取CMK的全局唯一标识
                     * @return KeyId CMK的全局唯一标识
                     */
                    std::string GetKeyId() const;

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                    /**
                     * 获取生成的DataKey的明文，该明文使用base64编码，用户需要使用base64解码得到明文
                     * @return Plaintext 生成的DataKey的明文，该明文使用base64编码，用户需要使用base64解码得到明文
                     */
                    std::string GetPlaintext() const;

                    /**
                     * 判断参数 Plaintext 是否已赋值
                     * @return Plaintext 是否已赋值
                     */
                    bool PlaintextHasBeenSet() const;

                    /**
                     * 获取DataKey加密后的密文，用户需要自行保存密文
                     * @return CiphertextBlob DataKey加密后的密文，用户需要自行保存密文
                     */
                    std::string GetCiphertextBlob() const;

                    /**
                     * 判断参数 CiphertextBlob 是否已赋值
                     * @return CiphertextBlob 是否已赋值
                     */
                    bool CiphertextBlobHasBeenSet() const;

                private:

                    /**
                     * CMK的全局唯一标识
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                    /**
                     * 生成的DataKey的明文，该明文使用base64编码，用户需要使用base64解码得到明文
                     */
                    std::string m_plaintext;
                    bool m_plaintextHasBeenSet;

                    /**
                     * DataKey加密后的密文，用户需要自行保存密文
                     */
                    std::string m_ciphertextBlob;
                    bool m_ciphertextBlobHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_GENERATEDATAKEYRESPONSE_H_
