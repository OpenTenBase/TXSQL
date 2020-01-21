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

#ifndef TENCENTCLOUD_CORE_EXECUTOR_H_
#define TENCENTCLOUD_CORE_EXECUTOR_H_

#include <atomic>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace TencentCloud
{
    class Runnable;
    class Executor
    {
    public:
        ~Executor();
        static Executor * GetInstance();

        void Submit(Runnable* task);
        bool Start();
        void Shutdown();
        void Wakeup();

    private:
        class Garbo
        {
        public:
            ~Garbo()
            {
                if (Executor::m_instance != nullptr)
                {
                    delete Executor::m_instance;
                    Executor::m_instance = nullptr;
                }
            }
        };

        Executor();
        static Executor *m_instance;
        static std::mutex m_instanceMutex;
        static Garbo m_garbo;

        std::atomic<bool> m_shutdown;
        std::mutex m_shutdownMutex;
        std::queue<Runnable*> m_tasksQueue;
        std::mutex m_tasksQueueMutex;
        std::thread m_thread;
        std::condition_variable m_cv;
        std::mutex m_cvMutex;
    };
}

#endif // !TENCENTCLOUD_CORE_EXECUTOR_H_
