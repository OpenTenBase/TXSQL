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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_DESCRIBEKEYRESPONSE_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_DESCRIBEKEYRESPONSE_H_

#include <string>
#include <vector>
#include <map>
#include <tencentcloud/core/AbstractModel.h>
#include <tencentcloud/kms/v20190118/model/KeyMetadata.h>


namespace TencentCloud
{
    namespace Kms
    {
        namespace V20190118
        {
            namespace Model
            {
                /**
                * DescribeKey返回参数结构体
                */
                class DescribeKeyResponse : public AbstractModel
                {
                public:
                    DescribeKeyResponse();
                    ~DescribeKeyResponse() = default;
                    CoreInternalOutcome Deserialize(const std::string &payload);


                    /**
                     * 获取密钥属性信息
注意：此字段可能返回 null，表示取不到有效值。
                     * @return KeyMetadata 密钥属性信息
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    KeyMetadata GetKeyMetadata() const;

                    /**
                     * 判断参数 KeyMetadata 是否已赋值
                     * @return KeyMetadata 是否已赋值
                     */
                    bool KeyMetadataHasBeenSet() const;

                private:

                    /**
                     * 密钥属性信息
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    KeyMetadata m_keyMetadata;
                    bool m_keyMetadataHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_DESCRIBEKEYRESPONSE_H_
