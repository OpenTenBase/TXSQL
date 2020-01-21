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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_LISTKEYSRESPONSE_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_LISTKEYSRESPONSE_H_

#include <string>
#include <vector>
#include <map>
#include <tencentcloud/core/AbstractModel.h>
#include <tencentcloud/kms/v20190118/model/Key.h>


namespace TencentCloud
{
    namespace Kms
    {
        namespace V20190118
        {
            namespace Model
            {
                /**
                * ListKeys返回参数结构体
                */
                class ListKeysResponse : public AbstractModel
                {
                public:
                    ListKeysResponse();
                    ~ListKeysResponse() = default;
                    CoreInternalOutcome Deserialize(const std::string &payload);


                    /**
                     * 获取CMK列表数组
注意：此字段可能返回 null，表示取不到有效值。
                     * @return Keys CMK列表数组
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    std::vector<Key> GetKeys() const;

                    /**
                     * 判断参数 Keys 是否已赋值
                     * @return Keys 是否已赋值
                     */
                    bool KeysHasBeenSet() const;

                    /**
                     * 获取CMK的总数量
                     * @return TotalCount CMK的总数量
                     */
                    uint64_t GetTotalCount() const;

                    /**
                     * 判断参数 TotalCount 是否已赋值
                     * @return TotalCount 是否已赋值
                     */
                    bool TotalCountHasBeenSet() const;

                private:

                    /**
                     * CMK列表数组
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    std::vector<Key> m_keys;
                    bool m_keysHasBeenSet;

                    /**
                     * CMK的总数量
                     */
                    uint64_t m_totalCount;
                    bool m_totalCountHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_LISTKEYSRESPONSE_H_
