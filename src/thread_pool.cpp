/*
 * This file is part of SQLogger.
 *
 * SQLogger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SQLogger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SQLogger. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 Sergey K. sergey[no_spam]@greenblit.com
 */

#include "thread_pool.h"

/**
 * @brief Constructs a ThreadPool with the specified number of threads.
 * @param numThreads The number of threads in the pool.
 */
ThreadPool::ThreadPool(size_t numThreads) : stop(false), tasksInProgress(0)
{
    for(size_t i = 0; i < numThreads; ++i)
    {
        workers.emplace_back([this]
        {
            while(true)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this]
                    {
                        return this->stop || !this->tasks.empty();
                    });

                    if(this->stop && this->tasks.empty())
                    {
                        return;
                    }

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    tasksInProgress++;
                }

                task();

                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    tasksInProgress--;
                    if(tasksInProgress == 0 && tasks.empty())
                    {
                        completionCondition.notify_all();
                    }
                }
            }
        });
    }
}

/**
 * @brief Destructor for ThreadPool. Stops all threads.
 */
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();

    for(std::thread & worker : workers)
    {
        worker.join();
    }
}

void ThreadPool::waitForCompletion()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    completionCondition.wait(lock, [this]
    {
        return tasks.empty() && tasksInProgress == 0;
    });
}

bool ThreadPool::isQueueEmpty()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    return tasks.empty() && tasksInProgress == 0;
}
