#include <iostream>

#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#include <functional>

#include <vector>
#include <queue>

namespace gbase
{

    class ThreadPool
    {
    public:
        // task
        ThreadPool(size_t numThreads);
        ~ThreadPool();

        void enqueue(std::function<void()> task);

    private:
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> queue;
        std::mutex mutexForQueue;
        std::condition_variable cond;
        bool stop;

        void workerThread();
    };

    ThreadPool::ThreadPool(size_t numThreads):stop(false)
    {
        for (size_t i = 0; i < numThreads; i++)
        {
            threads.emplace_back(&ThreadPool::workerThread, this);
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mutexForQueue);
            stop = true;
        }
        cond.notify_all();
        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void ThreadPool::enqueue(std::function<void()> message)
    {
        {
            std::unique_lock<std::mutex> lock(mutexForQueue);
            queue.push(std::move(message));
        }
        cond.notify_one();
    }

    void ThreadPool::workerThread()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutexForQueue);
                cond.wait(lock, [&]{return stop || !queue.empty();});
                if (stop && queue.empty())
                    return;
                message = std::move(queue.front());
                auto result = ME.match(message);
                queue.pop();
            }
            task();
            
        }
    }
}


// int main()
// {
//     ThreadPool pool(5);
//     for(int i = 0; i<10; i++)
//     {
//         pool.enqueue(
//             [i]{
//                 std::cout << "Task: "<< i << "-----> Therad ID: " << std::this_thread::get_id() << std::endl;
//             }
//         );
//     }
//     return 0;
// }