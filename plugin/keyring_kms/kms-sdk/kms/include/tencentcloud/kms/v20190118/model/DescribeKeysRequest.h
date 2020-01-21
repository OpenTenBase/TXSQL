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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_DESCRIBEKEYSREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_DESCRIBEKEYSREQUEST_H_

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
                * DescribeKeys请求参数结构体
                */
                class DescribeKeysRequest : public AbstractModel
                {
                public:
                    DescribeKeysRequest();
                    ~DescribeKeysRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取查询CMK的ID列表，批量查询一次最多支持100个KeyId
                     * @return KeyIds 查询CMK的ID列表，批量查询一次最多支持100个KeyId
                     */
                    std::vector<std::string> GetKeyIds() const;

                    /**
                     * 设置查询CMK的ID列表，批量查询一次最多支持100个KeyId
                     * @param KeyIds 查询CMK的ID列表，批量查询一次最多支持100个KeyId
                     */
                    void SetKeyIds(const std::vector<std::string>& _keyIds);

                    /**
                     * 判断参数 KeyIds 是否已赋值
                     * @return KeyIds 是否已赋值
                     */
                    bool KeyIdsHasBeenSet() const;

                private:

                    /**
                     * 查询CMK的ID列表，批量查询一次最多支持100个KeyId
                     */
                    std::vector<std::string> m_keyIds;
                    bool m_keyIdsHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_DESCRIBEKEYSREQUEST_H_
