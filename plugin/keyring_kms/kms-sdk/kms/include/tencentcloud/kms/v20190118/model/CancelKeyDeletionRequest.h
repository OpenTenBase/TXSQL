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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_CANCELKEYDELETIONREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_CANCELKEYDELETIONREQUEST_H_

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
                * CancelKeyDeletion请求参数结构体
                */
                class CancelKeyDeletionRequest : public AbstractModel
                {
                public:
                    CancelKeyDeletionRequest();
                    ~CancelKeyDeletionRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取需要被取消删除的CMK的唯一标志
                     * @return KeyId 需要被取消删除的CMK的唯一标志
                     */
                    std::string GetKeyId() const;

                    /**
                     * 设置需要被取消删除的CMK的唯一标志
                     * @param KeyId 需要被取消删除的CMK的唯一标志
                     */
                    void SetKeyId(const std::string& _keyId);

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                private:

                    /**
                     * 需要被取消删除的CMK的唯一标志
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_CANCELKEYDELETIONREQUEST_H_
