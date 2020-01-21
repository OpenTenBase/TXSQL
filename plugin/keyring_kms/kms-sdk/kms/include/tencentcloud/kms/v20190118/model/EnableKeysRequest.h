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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_ENABLEKEYSREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_ENABLEKEYSREQUEST_H_

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
                * EnableKeys请求参数结构体
                */
                class EnableKeysRequest : public AbstractModel
                {
                public:
                    EnableKeysRequest();
                    ~EnableKeysRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取需要批量启用的CMK Id 列表， CMK数量最大支持100
                     * @return KeyIds 需要批量启用的CMK Id 列表， CMK数量最大支持100
                     */
                    std::vector<std::string> GetKeyIds() const;

                    /**
                     * 设置需要批量启用的CMK Id 列表， CMK数量最大支持100
                     * @param KeyIds 需要批量启用的CMK Id 列表， CMK数量最大支持100
                     */
                    void SetKeyIds(const std::vector<std::string>& _keyIds);

                    /**
                     * 判断参数 KeyIds 是否已赋值
                     * @return KeyIds 是否已赋值
                     */
                    bool KeyIdsHasBeenSet() const;

                private:

                    /**
                     * 需要批量启用的CMK Id 列表， CMK数量最大支持100
                     */
                    std::vector<std::string> m_keyIds;
                    bool m_keyIdsHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_ENABLEKEYSREQUEST_H_
