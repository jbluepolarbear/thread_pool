/*
MIT License

Copyright(c) 2017 Jeremy Robert Anderson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef thread_pool_h
#define thread_pool_h
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

class thread_pool
{
public:
    using Result = std::function<void()>;
    using Task = std::function<Result()>;
    thread_pool(size_t numThreads)
        : mNumThreads(numThreads)
        , mRunning(true)
    {
        for (size_t i = 0; i < mNumThreads; ++i)
        {
            mThreads.emplace_back(std::thread(&thread_pool::threadLoop, this));
        }
    }

    ~thread_pool()
    {
        mRunning = false;
        mConditionVariable.notify_all();
        for (auto &thread : mThreads)
        {
            thread.join();
        }
    }

    void enqueue(const Task &task)
    {
        {
            std::lock_guard<std::mutex> guard(mTaskMutex);
            mTasks.push(task);
        }
        mConditionVariable.notify_one();
    }

    void update()
    {
        {
            std::lock_guard<std::mutex> guard(mResultMutex);
            while (!mResults.empty())
            {
                auto result = mResults.front();
                mResults.pop();
                result();
            }
        }
    }

private:
    void threadLoop()
    {
        while (mRunning)
        {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mTaskMutex);
                mConditionVariable.wait(lock, [this]() { return !mTasks.empty() || !mRunning; });
                if (!mRunning)
                {
                    break;
                }
                if (mTasks.empty())
                {
                    continue;
                }
                task = mTasks.front();
                mTasks.pop();
            }
            if (task != nullptr)
            {
                auto result = task();
                {
                    std::lock_guard<std::mutex> guard(mResultMutex);
                    mResults.push(result);
                }
            }
        }
    }

    std::mutex mTaskMutex;
    std::queue<Task> mTasks;
    std::mutex mResultMutex;
    std::queue<Result> mResults;
    std::atomic<bool> mRunning;
    std::vector<std::thread> mThreads;
    std::condition_variable mConditionVariable;

    const size_t mNumThreads;
};


#endif /* thread_pool_h */
