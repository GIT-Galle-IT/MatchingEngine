#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>

#include "gLockFreeQueue.h"

namespace gbase
{

    template<typename T>
    class MessageProcessor
    {
    public:
        MessageProcessor(size_t numThreads);
        ~MessageProcessor();

        void enqueue(const T& message);

    private:
        std::vector<std::thread> threads;
        LockFreeQueue<T> messageQueue;
        std::mutex mutexForQueue;
        std::condition_variable cond;
        std::atomic<bool> stop;

        void workerThread();
        void process(T& message);
    };
    
    template<typename T>
    MessageProcessor<T>::MessageProcessor(size_t numThreads):stop(false)
    {
        for (size_t i = 0; i < numThreads; i++)
        {
            threads.emplace_back(&MessageProcessor::workerThread, this);
        }
    }

    template<typename T>
    MessageProcessor<T>::~MessageProcessor()
    {
        stop = true;
        cond.notify_all();
        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    template<typename T>
    void MessageProcessor<T>::enqueue(const T& message)
    {
        {
            messageQueue.produce(std::move(message));
        }
        cond.notify_one();
    }

    template<typename T>
    void MessageProcessor<T>::workerThread()
    {
        while (true)
        {
            T message;
            cond.wait(lock, [&]{return stop || !messageQueue.empty();});
            if (stop && messageQueue.empty())
                return;
            messageQueue.consume(message);
            process(message);
        }
    }

    template<typename T>
    void MessageProcessor<T>::process(T& message)
    {
        // TODO: add the correct implemenation to process the message in ME

    }
}