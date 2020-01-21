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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_LISTKEYDETAILREQUEST_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_LISTKEYDETAILREQUEST_H_

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
                * ListKeyDetail请求参数结构体
                */
                class ListKeyDetailRequest : public AbstractModel
                {
                public:
                    ListKeyDetailRequest();
                    ~ListKeyDetailRequest() = default;
                    std::string ToJsonString() const;


                    /**
                     * 获取含义跟 SQL 查询的 Offset 一致，表示本次获取从按一定顺序排列数组的第 Offset 个元素开始，缺省为0
                     * @return Offset 含义跟 SQL 查询的 Offset 一致，表示本次获取从按一定顺序排列数组的第 Offset 个元素开始，缺省为0
                     */
                    uint64_t GetOffset() const;

                    /**
                     * 设置含义跟 SQL 查询的 Offset 一致，表示本次获取从按一定顺序排列数组的第 Offset 个元素开始，缺省为0
                     * @param Offset 含义跟 SQL 查询的 Offset 一致，表示本次获取从按一定顺序排列数组的第 Offset 个元素开始，缺省为0
                     */
                    void SetOffset(const uint64_t& _offset);

                    /**
                     * 判断参数 Offset 是否已赋值
                     * @return Offset 是否已赋值
                     */
                    bool OffsetHasBeenSet() const;

                    /**
                     * 获取含义跟 SQL 查询的 Limit 一致，表示本次获最多获取 Limit 个元素。缺省值为10，最大值为200
                     * @return Limit 含义跟 SQL 查询的 Limit 一致，表示本次获最多获取 Limit 个元素。缺省值为10，最大值为200
                     */
                    uint64_t GetLimit() const;

                    /**
                     * 设置含义跟 SQL 查询的 Limit 一致，表示本次获最多获取 Limit 个元素。缺省值为10，最大值为200
                     * @param Limit 含义跟 SQL 查询的 Limit 一致，表示本次获最多获取 Limit 个元素。缺省值为10，最大值为200
                     */
                    void SetLimit(const uint64_t& _limit);

                    /**
                     * 判断参数 Limit 是否已赋值
                     * @return Limit 是否已赋值
                     */
                    bool LimitHasBeenSet() const;

                    /**
                     * 获取根据创建者角色筛选，默认 0 表示用户自己创建的cmk， 1 表示授权其它云产品自动创建的cmk
                     * @return Role 根据创建者角色筛选，默认 0 表示用户自己创建的cmk， 1 表示授权其它云产品自动创建的cmk
                     */
                    uint64_t GetRole() const;

                    /**
                     * 设置根据创建者角色筛选，默认 0 表示用户自己创建的cmk， 1 表示授权其它云产品自动创建的cmk
                     * @param Role 根据创建者角色筛选，默认 0 表示用户自己创建的cmk， 1 表示授权其它云产品自动创建的cmk
                     */
                    void SetRole(const uint64_t& _role);

                    /**
                     * 判断参数 Role 是否已赋值
                     * @return Role 是否已赋值
                     */
                    bool RoleHasBeenSet() const;

                    /**
                     * 获取根据CMK创建时间排序， 0 表示按照降序排序，1表示按照升序排序
                     * @return OrderType 根据CMK创建时间排序， 0 表示按照降序排序，1表示按照升序排序
                     */
                    uint64_t GetOrderType() const;

                    /**
                     * 设置根据CMK创建时间排序， 0 表示按照降序排序，1表示按照升序排序
                     * @param OrderType 根据CMK创建时间排序， 0 表示按照降序排序，1表示按照升序排序
                     */
                    void SetOrderType(const uint64_t& _orderType);

                    /**
                     * 判断参数 OrderType 是否已赋值
                     * @return OrderType 是否已赋值
                     */
                    bool OrderTypeHasBeenSet() const;

                    /**
                     * 获取根据CMK状态筛选， 0表示全部CMK， 1 表示仅查询Enabled CMK， 2 表示仅查询Disabled CMK，3表示查询PendingDelete CMK(处于计划删除状态的Key)
                     * @return KeyState 根据CMK状态筛选， 0表示全部CMK， 1 表示仅查询Enabled CMK， 2 表示仅查询Disabled CMK，3表示查询PendingDelete CMK(处于计划删除状态的Key)
                     */
                    uint64_t GetKeyState() const;

                    /**
                     * 设置根据CMK状态筛选， 0表示全部CMK， 1 表示仅查询Enabled CMK， 2 表示仅查询Disabled CMK，3表示查询PendingDelete CMK(处于计划删除状态的Key)
                     * @param KeyState 根据CMK状态筛选， 0表示全部CMK， 1 表示仅查询Enabled CMK， 2 表示仅查询Disabled CMK，3表示查询PendingDelete CMK(处于计划删除状态的Key)
                     */
                    void SetKeyState(const uint64_t& _keyState);

                    /**
                     * 判断参数 KeyState 是否已赋值
                     * @return KeyState 是否已赋值
                     */
                    bool KeyStateHasBeenSet() const;

                    /**
                     * 获取根据KeyId或者Alias进行模糊匹配查询
                     * @return SearchKeyAlias 根据KeyId或者Alias进行模糊匹配查询
                     */
                    std::string GetSearchKeyAlias() const;

                    /**
                     * 设置根据KeyId或者Alias进行模糊匹配查询
                     * @param SearchKeyAlias 根据KeyId或者Alias进行模糊匹配查询
                     */
                    void SetSearchKeyAlias(const std::string& _searchKeyAlias);

                    /**
                     * 判断参数 SearchKeyAlias 是否已赋值
                     * @return SearchKeyAlias 是否已赋值
                     */
                    bool SearchKeyAliasHasBeenSet() const;

                private:

                    /**
                     * 含义跟 SQL 查询的 Offset 一致，表示本次获取从按一定顺序排列数组的第 Offset 个元素开始，缺省为0
                     */
                    uint64_t m_offset;
                    bool m_offsetHasBeenSet;

                    /**
                     * 含义跟 SQL 查询的 Limit 一致，表示本次获最多获取 Limit 个元素。缺省值为10，最大值为200
                     */
                    uint64_t m_limit;
                    bool m_limitHasBeenSet;

                    /**
                     * 根据创建者角色筛选，默认 0 表示用户自己创建的cmk， 1 表示授权其它云产品自动创建的cmk
                     */
                    uint64_t m_role;
                    bool m_roleHasBeenSet;

                    /**
                     * 根据CMK创建时间排序， 0 表示按照降序排序，1表示按照升序排序
                     */
                    uint64_t m_orderType;
                    bool m_orderTypeHasBeenSet;

                    /**
                     * 根据CMK状态筛选， 0表示全部CMK， 1 表示仅查询Enabled CMK， 2 表示仅查询Disabled CMK，3表示查询PendingDelete CMK(处于计划删除状态的Key)
                     */
                    uint64_t m_keyState;
                    bool m_keyStateHasBeenSet;

                    /**
                     * 根据KeyId或者Alias进行模糊匹配查询
                     */
                    std::string m_searchKeyAlias;
                    bool m_searchKeyAliasHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_LISTKEYDETAILREQUEST_H_
