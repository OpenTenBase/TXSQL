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

#ifndef TENCENTCLOUD_KMS_V20190118_MODEL_KEYMETADATA_H_
#define TENCENTCLOUD_KMS_V20190118_MODEL_KEYMETADATA_H_

#include <string>
#include <vector>
#include <map>
#include <tencentcloud/core/utils/rapidjson/document.h>
#include <tencentcloud/core/utils/rapidjson/writer.h>
#include <tencentcloud/core/utils/rapidjson/stringbuffer.h>
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
                * CMK属性信息
                */
                class KeyMetadata : public AbstractModel
                {
                public:
                    KeyMetadata();
                    ~KeyMetadata() = default;
                    void ToJsonObject(rapidjson::Value &value, rapidjson::Document::AllocatorType& allocator) const;
                    CoreInternalOutcome Deserialize(const rapidjson::Value &value);


                    /**
                     * 获取CMK的全局唯一标识
                     * @return KeyId CMK的全局唯一标识
                     */
                    std::string GetKeyId() const;

                    /**
                     * 设置CMK的全局唯一标识
                     * @param KeyId CMK的全局唯一标识
                     */
                    void SetKeyId(const std::string& _keyId);

                    /**
                     * 判断参数 KeyId 是否已赋值
                     * @return KeyId 是否已赋值
                     */
                    bool KeyIdHasBeenSet() const;

                    /**
                     * 获取作为密钥更容易辨识，更容易被人看懂的别名
                     * @return Alias 作为密钥更容易辨识，更容易被人看懂的别名
                     */
                    std::string GetAlias() const;

                    /**
                     * 设置作为密钥更容易辨识，更容易被人看懂的别名
                     * @param Alias 作为密钥更容易辨识，更容易被人看懂的别名
                     */
                    void SetAlias(const std::string& _alias);

                    /**
                     * 判断参数 Alias 是否已赋值
                     * @return Alias 是否已赋值
                     */
                    bool AliasHasBeenSet() const;

                    /**
                     * 获取密钥创建时间
                     * @return CreateTime 密钥创建时间
                     */
                    uint64_t GetCreateTime() const;

                    /**
                     * 设置密钥创建时间
                     * @param CreateTime 密钥创建时间
                     */
                    void SetCreateTime(const uint64_t& _createTime);

                    /**
                     * 判断参数 CreateTime 是否已赋值
                     * @return CreateTime 是否已赋值
                     */
                    bool CreateTimeHasBeenSet() const;

                    /**
                     * 获取CMK的描述
                     * @return Description CMK的描述
                     */
                    std::string GetDescription() const;

                    /**
                     * 设置CMK的描述
                     * @param Description CMK的描述
                     */
                    void SetDescription(const std::string& _description);

                    /**
                     * 判断参数 Description 是否已赋值
                     * @return Description 是否已赋值
                     */
                    bool DescriptionHasBeenSet() const;

                    /**
                     * 获取CMK的状态， Enabled 或者 Disabled 或者PendingDelete状态
                     * @return KeyState CMK的状态， Enabled 或者 Disabled 或者PendingDelete状态
                     */
                    std::string GetKeyState() const;

                    /**
                     * 设置CMK的状态， Enabled 或者 Disabled 或者PendingDelete状态
                     * @param KeyState CMK的状态， Enabled 或者 Disabled 或者PendingDelete状态
                     */
                    void SetKeyState(const std::string& _keyState);

                    /**
                     * 判断参数 KeyState 是否已赋值
                     * @return KeyState 是否已赋值
                     */
                    bool KeyStateHasBeenSet() const;

                    /**
                     * 获取CMK用途，当前是 ENCRYPT_DECRYPT
                     * @return KeyUsage CMK用途，当前是 ENCRYPT_DECRYPT
                     */
                    std::string GetKeyUsage() const;

                    /**
                     * 设置CMK用途，当前是 ENCRYPT_DECRYPT
                     * @param KeyUsage CMK用途，当前是 ENCRYPT_DECRYPT
                     */
                    void SetKeyUsage(const std::string& _keyUsage);

                    /**
                     * 判断参数 KeyUsage 是否已赋值
                     * @return KeyUsage 是否已赋值
                     */
                    bool KeyUsageHasBeenSet() const;

                    /**
                     * 获取CMK类型，当前为 1 普通类型
                     * @return Type CMK类型，当前为 1 普通类型
                     */
                    int64_t GetType() const;

                    /**
                     * 设置CMK类型，当前为 1 普通类型
                     * @param Type CMK类型，当前为 1 普通类型
                     */
                    void SetType(const int64_t& _type);

                    /**
                     * 判断参数 Type 是否已赋值
                     * @return Type 是否已赋值
                     */
                    bool TypeHasBeenSet() const;

                    /**
                     * 获取创建者
                     * @return CreatorUin 创建者
                     */
                    uint64_t GetCreatorUin() const;

                    /**
                     * 设置创建者
                     * @param CreatorUin 创建者
                     */
                    void SetCreatorUin(const uint64_t& _creatorUin);

                    /**
                     * 判断参数 CreatorUin 是否已赋值
                     * @return CreatorUin 是否已赋值
                     */
                    bool CreatorUinHasBeenSet() const;

                    /**
                     * 获取是否开启了密钥轮换功能
                     * @return KeyRotationEnabled 是否开启了密钥轮换功能
                     */
                    bool GetKeyRotationEnabled() const;

                    /**
                     * 设置是否开启了密钥轮换功能
                     * @param KeyRotationEnabled 是否开启了密钥轮换功能
                     */
                    void SetKeyRotationEnabled(const bool& _keyRotationEnabled);

                    /**
                     * 判断参数 KeyRotationEnabled 是否已赋值
                     * @return KeyRotationEnabled 是否已赋值
                     */
                    bool KeyRotationEnabledHasBeenSet() const;

                    /**
                     * 获取CMK的创建者，用户创建的为 user，授权各云产品自动创建的为对应的产品名
                     * @return Owner CMK的创建者，用户创建的为 user，授权各云产品自动创建的为对应的产品名
                     */
                    std::string GetOwner() const;

                    /**
                     * 设置CMK的创建者，用户创建的为 user，授权各云产品自动创建的为对应的产品名
                     * @param Owner CMK的创建者，用户创建的为 user，授权各云产品自动创建的为对应的产品名
                     */
                    void SetOwner(const std::string& _owner);

                    /**
                     * 判断参数 Owner 是否已赋值
                     * @return Owner 是否已赋值
                     */
                    bool OwnerHasBeenSet() const;

                    /**
                     * 获取在密钥轮换开启状态下，下次轮换的时间
                     * @return NextRotateTime 在密钥轮换开启状态下，下次轮换的时间
                     */
                    uint64_t GetNextRotateTime() const;

                    /**
                     * 设置在密钥轮换开启状态下，下次轮换的时间
                     * @param NextRotateTime 在密钥轮换开启状态下，下次轮换的时间
                     */
                    void SetNextRotateTime(const uint64_t& _nextRotateTime);

                    /**
                     * 判断参数 NextRotateTime 是否已赋值
                     * @return NextRotateTime 是否已赋值
                     */
                    bool NextRotateTimeHasBeenSet() const;

                    /**
                     * 获取计划删除的时间
注意：此字段可能返回 null，表示取不到有效值。
                     * @return DeletionDate 计划删除的时间
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    uint64_t GetDeletionDate() const;

                    /**
                     * 设置计划删除的时间
注意：此字段可能返回 null，表示取不到有效值。
                     * @param DeletionDate 计划删除的时间
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    void SetDeletionDate(const uint64_t& _deletionDate);

                    /**
                     * 判断参数 DeletionDate 是否已赋值
                     * @return DeletionDate 是否已赋值
                     */
                    bool DeletionDateHasBeenSet() const;

                private:

                    /**
                     * CMK的全局唯一标识
                     */
                    std::string m_keyId;
                    bool m_keyIdHasBeenSet;

                    /**
                     * 作为密钥更容易辨识，更容易被人看懂的别名
                     */
                    std::string m_alias;
                    bool m_aliasHasBeenSet;

                    /**
                     * 密钥创建时间
                     */
                    uint64_t m_createTime;
                    bool m_createTimeHasBeenSet;

                    /**
                     * CMK的描述
                     */
                    std::string m_description;
                    bool m_descriptionHasBeenSet;

                    /**
                     * CMK的状态， Enabled 或者 Disabled 或者PendingDelete状态
                     */
                    std::string m_keyState;
                    bool m_keyStateHasBeenSet;

                    /**
                     * CMK用途，当前是 ENCRYPT_DECRYPT
                     */
                    std::string m_keyUsage;
                    bool m_keyUsageHasBeenSet;

                    /**
                     * CMK类型，当前为 1 普通类型
                     */
                    int64_t m_type;
                    bool m_typeHasBeenSet;

                    /**
                     * 创建者
                     */
                    uint64_t m_creatorUin;
                    bool m_creatorUinHasBeenSet;

                    /**
                     * 是否开启了密钥轮换功能
                     */
                    bool m_keyRotationEnabled;
                    bool m_keyRotationEnabledHasBeenSet;

                    /**
                     * CMK的创建者，用户创建的为 user，授权各云产品自动创建的为对应的产品名
                     */
                    std::string m_owner;
                    bool m_ownerHasBeenSet;

                    /**
                     * 在密钥轮换开启状态下，下次轮换的时间
                     */
                    uint64_t m_nextRotateTime;
                    bool m_nextRotateTimeHasBeenSet;

                    /**
                     * 计划删除的时间
注意：此字段可能返回 null，表示取不到有效值。
                     */
                    uint64_t m_deletionDate;
                    bool m_deletionDateHasBeenSet;

                };
            }
        }
    }
}

#endif // !TENCENTCLOUD_KMS_V20190118_MODEL_KEYMETADATA_H_
