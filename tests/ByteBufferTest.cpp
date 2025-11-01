// #include "../src/matching/MatchingEngine.hpp"
#include "Setup.hpp"
#include <ByteBuffer.hpp>
#include <ObjectPool.hpp>
#include <demos/message.h>
#include <chrono>
#include <logging/gLog.h>
#include <print>
using namespace std::chrono;

TEST_F(BaseTest, ByteBuffer_Test)
{
    // Message message{8888, 1000, "Hello, Server request from clientele", true, 10000000};
    // GLOG_DEBUG_L1("Size of request: {}", sizeof(message));

    // std::stringstream oss;
    // message.serialize(oss);
    // const char *some_string = "oss.str().c_str()";
    // // ByteBuffer bb;
    // bb.allocate(some_string, strlen(some_string));
    // bb.release();

    EXPECT_TRUE(true);
}

TEST_F(BaseTest, ObjectPool_Test)
{
    class MyInt
    {
    public:
        MyInt() {};
        MyInt(int i) : value{i} {};
        unsigned long long value;
    };

    constexpr unsigned long long SIZE_OF_POOL = 10 ;
    GLOG_INFO("pool size {}", SIZE_OF_POOL)
    std::println("pool size {}", SIZE_OF_POOL);
    gbase::ObjectPool<MyInt, SIZE_OF_POOL> objPool;
    
    auto start = high_resolution_clock::now();
    // std::println("sizeof(object) {}", sizeof(MyInt));

    for (unsigned long long i = 0; i < SIZE_OF_POOL; i++)
    {
        MyInt* a = objPool.allocate();
        a->value = i;
    }
    objPool.release();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(stop - start);
    std::cout << duration.count()  << std::endl; // 0 seconds
    int pool_allocation_time = duration.count();

    start = high_resolution_clock::now();
    for (unsigned long long i = 0; i < SIZE_OF_POOL; i++)
    {
        MyInt* a = new MyInt;
        a->value = i;
    }

    stop = high_resolution_clock::now();
    duration = duration_cast<nanoseconds>(stop - start);
    std::cout << duration.count()  << std::endl; // 0 seconds
    std::println("How many times faster object pool allocation = {}", (double) duration.count() / (double)pool_allocation_time);

    EXPECT_TRUE(true);
}
