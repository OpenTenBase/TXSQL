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

#include <tencentcloud/core/Sign.h>
#include <tencentcloud/core/utils/Utils.h>

using namespace TencentCloud;
using namespace std;

string Sign::Tc3Sign(const string &credDate, const string &serviceName, const string &signStr)
{
    string kKey = "TC3"+this->m_secretKey;
    string kDate = Utils::HmacSha256(kKey, credDate);
    string kService = Utils::HmacSha256(kDate, serviceName);
    string kSigning = Utils::HmacSha256(kService, "tc3_request");
    return Utils::HexEncode(Utils::HmacSha256(kSigning, signStr));
}
