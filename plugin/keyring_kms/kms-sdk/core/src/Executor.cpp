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

#include <tencentcloud/core/Executor.h>
#include <tencentcloud/core/Runnable.h>
#include <tencentcloud/core/http/HttpClient.h>

using namespace TencentCloud;
using namespace std;

Executor *Executor::m_instance = nullptr;
mutex Executor::m_instanceMutex;
Executor::Garbo Executor::m_garbo;

Executor::Executor() :
    m_shutdown(true)
{
}

Executor::~Executor()
{
    Shutdown();
}

Executor * Executor::GetInstance()
{
    if (m_instance == nullptr)
    {
        lock_guard<mutex> lock(m_instanceMutex);
        if (m_instance == nullptr)
        {
            m_instance = new Executor;
        }
    }
    return m_instance;
}

bool Executor::Start()
{
    lock_guard<mutex> lk(m_shutdownMutex);

    if (!m_shutdown)
        return true;

    HttpClient::InitGlobalState();

    auto threadMain = [this]()
    {
        while (!m_shutdown)
        {
            while (!m_tasksQueue.empty())
            {
                Runnable *task = nullptr;
                {
                    lock_guard<mutex> lock(m_tasksQueueMutex);
                    if (!m_tasksQueue.empty())
                    {
                        task = m_tasksQueue.front();
                        m_tasksQueue.pop();
                    }
                }

                if (task) {
                    task->Run();
                    delete task;
                }
            }

            if (!m_shutdown)
            {
                unique_lock<mutex> lock(m_cvMutex);
                m_cv.wait(lock);
            }
        }
    };

    m_shutdown = false;
    m_thread = thread(threadMain);
    return true;
}

void Executor::Submit(Runnable* task)
{
    if (m_shutdown)
        return;

    lock_guard<mutex> lock(m_tasksQueueMutex);
    m_tasksQueue.push(task);
    Wakeup();
}

void Executor::Wakeup()
{
    unique_lock<mutex> lock(m_cvMutex);
    m_cv.notify_one();
}

void Executor::Shutdown()
{
    lock_guard<mutex> lk(m_shutdownMutex);

    if (m_shutdown)
        return;

    HttpClient::CleanupGlobalState();

    {
        lock_guard<mutex> lock(m_tasksQueueMutex);
        while (m_tasksQueue.size() > 0)
        {
            auto task = m_tasksQueue.front();
            delete task;
            m_tasksQueue.pop();
        }
    }

    m_shutdown = true;
    Wakeup();

    if (m_thread.joinable())
        m_thread.join();
}
