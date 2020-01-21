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

#ifndef TENCENTCLOUD_CORE_RUNNABLE_H_
#define TENCENTCLOUD_CORE_RUNNABLE_H_

#include <functional>

namespace TencentCloud
{
    class Runnable
    {
    public:
        explicit Runnable(const std::function<void()> func) :
            m_func(func)
        {
        }

        void Run() const
        {
            m_func();
        }

    private:
        std::function<void()> m_func;
    };

}

#endif // !TENCENTCLOUD_CORE_RUNNABLE_H_
