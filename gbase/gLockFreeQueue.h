#include <iostream>
#include <vector>
#include <atomic>
#include <thread>

constexpr size_t BUFFER_SIZE = 1024;
constexpr size_t INDEX_MASK = BUFFER_SIZE - 1;

// TODO: try using a memory mapped file instead of a queue for sequencing

namespace gbase
{

    template<typename T>
    class LockFreeQueue {
    public:
        LockFreeQueue() : buffer(BUFFER_SIZE), producerSequence(0), consumerSequence(0) {}

        // Publishes a message
        bool produce(const T& obj) {
            size_t nextSeq = producerSequence.load(std::memory_order_relaxed) + 1; // no sync, doent matter as publish, consume handle this, okay to use acquire here as well, might add bit latency
            if (nextSeq - consumerSequence.load(std::memory_order_acquire) > BUFFER_SIZE) { //prior writes are visible
                return false; // If the buffer is full returns false, can retry untill true from producer end
            }
            buffer[nextSeq & INDEX_MASK] = obj;
            producerSequence.store(nextSeq, std::memory_order_release); //ensure that writes happens before order acquire
            return true;
        }

        // Retrieves a message
        bool consume(T& obj) {
            size_t nextSeq = consumerSequence.load(std::memory_order_relaxed) + 1;
            if (nextSeq > producerSequence.load(std::memory_order_acquire)) { //prior writes are visible
                return false; // Nothing avaibale to consume
                // TODO : Try notifying once the queue become not empty, to the consumer 
            }
            obj = buffer[nextSeq & INDEX_MASK];
            consumerSequence.store(nextSeq, std::memory_order_release); //ensure that writes happens before order acquire
            return true;
        }

    private:
        std::vector<T> buffer;
        std::atomic<size_t> producerSequence;
        std::atomic<size_t> consumerSequence;
    };

}