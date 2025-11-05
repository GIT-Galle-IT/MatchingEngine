// #include "../src/matching/MatchingEngine.hpp"
#include "Setup.hpp"
#include <ByteBuffer.hpp>
#include <ObjectPool.hpp>
#include <demos/message.h>
#include <chrono>
#include <logging/gLog.h>
#include <print>

TEST_F(BaseTest, ByteBuffer_validation_Allocate_Test)
{
    const auto some_string = "some string with special characters"" 12345677 \u3000d !@#$%\n/n'\'n''''";
    gbase::ByteBuffer<std::byte> bb;
    bb.allocate(some_string, strlen(some_string));
    const auto buffer_ptr = bb.get();
    for (unsigned long i = 0; i < strlen(some_string); i++) {
        EXPECT_EQ(some_string[i], static_cast<char>(buffer_ptr.get()[i]));
    }

    bb.release();
}

TEST_F(BaseTest, ByteBuffer_validation_Append_Test)
{
    const auto some_string = "some string with special characters"" 12345677 \u3000d !@#$%\n/n'\'n''''";
    gbase::ByteBuffer<std::byte> bb;
    bb.append(some_string, strlen(some_string));
    const auto buffer_ptr = bb.get();
    for (unsigned long i = 0; i < strlen(some_string); i++) {
        EXPECT_EQ(some_string[i], static_cast<char>(buffer_ptr.get()[i]));
    }

    // appending
    bb.append(some_string, strlen(some_string));
    const auto buffer_ptr_appended = bb.get();
    for (unsigned long i = 0; i < 2*strlen(some_string); i++) {
        EXPECT_EQ(some_string[i%strlen(some_string)], static_cast<char>(buffer_ptr_appended.get()[i]));
    }

    // reallocate and ignore previous bytes
    const auto some_string_a = "abcef";
    bb.allocate(some_string_a, strlen(some_string_a));
    const auto buffer_ptr_reallocated = bb.get();
    for (unsigned long i = 0; i < strlen(some_string_a); i++) {
        EXPECT_EQ(some_string_a[i], static_cast<char>(buffer_ptr_reallocated.get()[i]));
    }

    bb.release();
}
