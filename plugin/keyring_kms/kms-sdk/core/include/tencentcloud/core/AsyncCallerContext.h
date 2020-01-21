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

#ifndef TENCENTCLOUD_CORE_ASYNCCALLERCONTEXT_H_
#define TENCENTCLOUD_CORE_ASYNCCALLERCONTEXT_H_

#include <string>

namespace TencentCloud
{
  class AsyncCallerContext
  {
  public:
    AsyncCallerContext();
    explicit AsyncCallerContext(const std::string &uuid);
    virtual ~AsyncCallerContext();

    std::string GetUuid()const;
    void SetUuid(const std::string &uuid);

  private:
    std::string m_uuid;
  };
}
#endif // !TENCENTCLOUD_CORE_ASYNCCALLERCONTEXT_H_
