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

#include <tencentcloud/kms/v20190118/model/KeyMetadata.h>

using TencentCloud::CoreInternalOutcome;
using namespace TencentCloud::Kms::V20190118::Model;
using namespace rapidjson;
using namespace std;

KeyMetadata::KeyMetadata() :
    m_keyIdHasBeenSet(false),
    m_aliasHasBeenSet(false),
    m_createTimeHasBeenSet(false),
    m_descriptionHasBeenSet(false),
    m_keyStateHasBeenSet(false),
    m_keyUsageHasBeenSet(false),
    m_typeHasBeenSet(false),
    m_creatorUinHasBeenSet(false),
    m_keyRotationEnabledHasBeenSet(false),
    m_ownerHasBeenSet(false),
    m_nextRotateTimeHasBeenSet(false),
    m_deletionDateHasBeenSet(false)
{
}

CoreInternalOutcome KeyMetadata::Deserialize(const Value &value)
{
    string requestId = "";


    if (value.HasMember("KeyId") && !value["KeyId"].IsNull())
    {
        if (!value["KeyId"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.KeyId` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyId = string(value["KeyId"].GetString());
        m_keyIdHasBeenSet = true;
    }

    if (value.HasMember("Alias") && !value["Alias"].IsNull())
    {
        if (!value["Alias"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.Alias` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_alias = string(value["Alias"].GetString());
        m_aliasHasBeenSet = true;
    }

    if (value.HasMember("CreateTime") && !value["CreateTime"].IsNull())
    {
        if (!value["CreateTime"].IsUint64())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.CreateTime` IsUint64=false incorrectly").SetRequestId(requestId));
        }
        m_createTime = value["CreateTime"].GetUint64();
        m_createTimeHasBeenSet = true;
    }

    if (value.HasMember("Description") && !value["Description"].IsNull())
    {
        if (!value["Description"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.Description` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_description = string(value["Description"].GetString());
        m_descriptionHasBeenSet = true;
    }

    if (value.HasMember("KeyState") && !value["KeyState"].IsNull())
    {
        if (!value["KeyState"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.KeyState` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyState = string(value["KeyState"].GetString());
        m_keyStateHasBeenSet = true;
    }

    if (value.HasMember("KeyUsage") && !value["KeyUsage"].IsNull())
    {
        if (!value["KeyUsage"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.KeyUsage` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_keyUsage = string(value["KeyUsage"].GetString());
        m_keyUsageHasBeenSet = true;
    }

    if (value.HasMember("Type") && !value["Type"].IsNull())
    {
        if (!value["Type"].IsInt64())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.Type` IsInt64=false incorrectly").SetRequestId(requestId));
        }
        m_type = value["Type"].GetInt64();
        m_typeHasBeenSet = true;
    }

    if (value.HasMember("CreatorUin") && !value["CreatorUin"].IsNull())
    {
        if (!value["CreatorUin"].IsUint64())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.CreatorUin` IsUint64=false incorrectly").SetRequestId(requestId));
        }
        m_creatorUin = value["CreatorUin"].GetUint64();
        m_creatorUinHasBeenSet = true;
    }

    if (value.HasMember("KeyRotationEnabled") && !value["KeyRotationEnabled"].IsNull())
    {
        if (!value["KeyRotationEnabled"].IsBool())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.KeyRotationEnabled` IsBool=false incorrectly").SetRequestId(requestId));
        }
        m_keyRotationEnabled = value["KeyRotationEnabled"].GetBool();
        m_keyRotationEnabledHasBeenSet = true;
    }

    if (value.HasMember("Owner") && !value["Owner"].IsNull())
    {
        if (!value["Owner"].IsString())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.Owner` IsString=false incorrectly").SetRequestId(requestId));
        }
        m_owner = string(value["Owner"].GetString());
        m_ownerHasBeenSet = true;
    }

    if (value.HasMember("NextRotateTime") && !value["NextRotateTime"].IsNull())
    {
        if (!value["NextRotateTime"].IsUint64())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.NextRotateTime` IsUint64=false incorrectly").SetRequestId(requestId));
        }
        m_nextRotateTime = value["NextRotateTime"].GetUint64();
        m_nextRotateTimeHasBeenSet = true;
    }

    if (value.HasMember("DeletionDate") && !value["DeletionDate"].IsNull())
    {
        if (!value["DeletionDate"].IsUint64())
        {
            return CoreInternalOutcome(Error("response `KeyMetadata.DeletionDate` IsUint64=false incorrectly").SetRequestId(requestId));
        }
        m_deletionDate = value["DeletionDate"].GetUint64();
        m_deletionDateHasBeenSet = true;
    }


    return CoreInternalOutcome(true);
}

void KeyMetadata::ToJsonObject(Value &value, Document::AllocatorType& allocator) const
{

    if (m_keyIdHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "KeyId";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_keyId.c_str(), allocator).Move(), allocator);
    }

    if (m_aliasHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Alias";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_alias.c_str(), allocator).Move(), allocator);
    }

    if (m_createTimeHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "CreateTime";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, m_createTime, allocator);
    }

    if (m_descriptionHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Description";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_description.c_str(), allocator).Move(), allocator);
    }

    if (m_keyStateHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "KeyState";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_keyState.c_str(), allocator).Move(), allocator);
    }

    if (m_keyUsageHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "KeyUsage";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_keyUsage.c_str(), allocator).Move(), allocator);
    }

    if (m_typeHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Type";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, m_type, allocator);
    }

    if (m_creatorUinHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "CreatorUin";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, m_creatorUin, allocator);
    }

    if (m_keyRotationEnabledHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "KeyRotationEnabled";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, m_keyRotationEnabled, allocator);
    }

    if (m_ownerHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "Owner";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, Value(m_owner.c_str(), allocator).Move(), allocator);
    }

    if (m_nextRotateTimeHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "NextRotateTime";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, m_nextRotateTime, allocator);
    }

    if (m_deletionDateHasBeenSet)
    {
        Value iKey(kStringType);
        string key = "DeletionDate";
        iKey.SetString(key.c_str(), allocator);
        value.AddMember(iKey, m_deletionDate, allocator);
    }

}


string KeyMetadata::GetKeyId() const
{
    return m_keyId;
}

void KeyMetadata::SetKeyId(const string& _keyId)
{
    m_keyId = _keyId;
    m_keyIdHasBeenSet = true;
}

bool KeyMetadata::KeyIdHasBeenSet() const
{
    return m_keyIdHasBeenSet;
}

string KeyMetadata::GetAlias() const
{
    return m_alias;
}

void KeyMetadata::SetAlias(const string& _alias)
{
    m_alias = _alias;
    m_aliasHasBeenSet = true;
}

bool KeyMetadata::AliasHasBeenSet() const
{
    return m_aliasHasBeenSet;
}

uint64_t KeyMetadata::GetCreateTime() const
{
    return m_createTime;
}

void KeyMetadata::SetCreateTime(const uint64_t& _createTime)
{
    m_createTime = _createTime;
    m_createTimeHasBeenSet = true;
}

bool KeyMetadata::CreateTimeHasBeenSet() const
{
    return m_createTimeHasBeenSet;
}

string KeyMetadata::GetDescription() const
{
    return m_description;
}

void KeyMetadata::SetDescription(const string& _description)
{
    m_description = _description;
    m_descriptionHasBeenSet = true;
}

bool KeyMetadata::DescriptionHasBeenSet() const
{
    return m_descriptionHasBeenSet;
}

string KeyMetadata::GetKeyState() const
{
    return m_keyState;
}

void KeyMetadata::SetKeyState(const string& _keyState)
{
    m_keyState = _keyState;
    m_keyStateHasBeenSet = true;
}

bool KeyMetadata::KeyStateHasBeenSet() const
{
    return m_keyStateHasBeenSet;
}

string KeyMetadata::GetKeyUsage() const
{
    return m_keyUsage;
}

void KeyMetadata::SetKeyUsage(const string& _keyUsage)
{
    m_keyUsage = _keyUsage;
    m_keyUsageHasBeenSet = true;
}

bool KeyMetadata::KeyUsageHasBeenSet() const
{
    return m_keyUsageHasBeenSet;
}

int64_t KeyMetadata::GetType() const
{
    return m_type;
}

void KeyMetadata::SetType(const int64_t& _type)
{
    m_type = _type;
    m_typeHasBeenSet = true;
}

bool KeyMetadata::TypeHasBeenSet() const
{
    return m_typeHasBeenSet;
}

uint64_t KeyMetadata::GetCreatorUin() const
{
    return m_creatorUin;
}

void KeyMetadata::SetCreatorUin(const uint64_t& _creatorUin)
{
    m_creatorUin = _creatorUin;
    m_creatorUinHasBeenSet = true;
}

bool KeyMetadata::CreatorUinHasBeenSet() const
{
    return m_creatorUinHasBeenSet;
}

bool KeyMetadata::GetKeyRotationEnabled() const
{
    return m_keyRotationEnabled;
}

void KeyMetadata::SetKeyRotationEnabled(const bool& _keyRotationEnabled)
{
    m_keyRotationEnabled = _keyRotationEnabled;
    m_keyRotationEnabledHasBeenSet = true;
}

bool KeyMetadata::KeyRotationEnabledHasBeenSet() const
{
    return m_keyRotationEnabledHasBeenSet;
}

string KeyMetadata::GetOwner() const
{
    return m_owner;
}

void KeyMetadata::SetOwner(const string& _owner)
{
    m_owner = _owner;
    m_ownerHasBeenSet = true;
}

bool KeyMetadata::OwnerHasBeenSet() const
{
    return m_ownerHasBeenSet;
}

uint64_t KeyMetadata::GetNextRotateTime() const
{
    return m_nextRotateTime;
}

void KeyMetadata::SetNextRotateTime(const uint64_t& _nextRotateTime)
{
    m_nextRotateTime = _nextRotateTime;
    m_nextRotateTimeHasBeenSet = true;
}

bool KeyMetadata::NextRotateTimeHasBeenSet() const
{
    return m_nextRotateTimeHasBeenSet;
}

uint64_t KeyMetadata::GetDeletionDate() const
{
    return m_deletionDate;
}

void KeyMetadata::SetDeletionDate(const uint64_t& _deletionDate)
{
    m_deletionDate = _deletionDate;
    m_deletionDateHasBeenSet = true;
}

bool KeyMetadata::DeletionDateHasBeenSet() const
{
    return m_deletionDateHasBeenSet;
}

