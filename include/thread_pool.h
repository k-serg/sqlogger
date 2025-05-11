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

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * @class ThreadPool
 * @brief A thread pool for managing and executing tasks concurrently.
 */
class ThreadPool
{
    public:
        /**
         * @brief Constructs a ThreadPool with the specified number of threads.
         * @param numThreads The number of threads in the pool.
         */
        ThreadPool(size_t numThreads);

        /**
         * @brief Destructor for ThreadPool. Stops all threads.
         */
        ~ThreadPool();

        /**
         * @brief Enqueues a task to be executed by the ThreadPool.
         * @param task The task to be executed.
         */
        template <class F>
        void enqueue(F&& task)
        {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                tasks.emplace(std::forward<F>(task));
            }
            condition.notify_one();
        }

        /**
        * @brief Waits for all tasks to complete.
        */
        void waitForCompletion();

        /**
        * @brief Checks if the task queue is empty and no tasks are in progress.
        * @return True if the queue is empty and no tasks are in progress, false otherwise.
        */
        bool isQueueEmpty();

    private:
        std::vector<std::thread> workers; /**< Worker threads. */
        std::queue<std::function<void()>> tasks; /**< Task queue. */

        std::mutex queueMutex; /**< Mutex for synchronizing access to the task queue. */
        std::condition_variable condition; /**< Condition variable for task notification. */
        std::condition_variable completionCondition;

        std::atomic<bool> stop; /**< Flag to stop the ThreadPool. */
        std::atomic<size_t> tasksInProgress;
};

#endif // THREAD_POOL_H