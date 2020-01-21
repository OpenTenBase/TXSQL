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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_UPDATEKEYDESCRIPTIONREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_UPDATEKEYDESCRIPTIONREQUEST_H_

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
                * UpdateKeyDescription请求参数结构体
                */
                class UpdateKeyDescriptionRequest : public AbstractModel
                {
                public:
                    UpdateKeyDescriptionRequest();
                    ~UpdateKeyDescriptionRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取新的描述信息，最大支持1024字节
                     * @return Description 新的描述信息，最大支持1024字节
                     */
                    std::string GetDescription() const;

                    /**
                     * 设置新的描述信息，最大支持1024字节
                     * @param Description 新的描述信息，最大支持1024字节
                     */
                    void SetDescription(const std::string& _description);

                    /**
                     * 判断参数 Description 是否已赋值
                     * @return Description 是否已赋值
                     */
                    bool DescriptionHasBeenSet() const;

                    /**
                     * 获取需要修改描述信息的的CMK ID
                     * @return KeyId 需要修改描述信息的的CMK ID
                     */
                    std::string GetKeyId() const;

                    /**
                     * 设置需要修改描述信息的的CMK ID
                     * @param KeyId 需要修改描述信息的的CMK ID
                     */
                    void SetKeyId(const std::string& _keyId);

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                private:

                    /**
                     * 新的描述信息，最大支持1024字节
                     */
                    std::string m_description;
                    bool m_descriptionHasBeenSet;

                    /**
                     * 需要修改描述信息的的CMK ID
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_UPDATEKEYDESCRIPTIONREQUEST_H_
