// #include "../src/matching/MatchingEngine.hpp"
#include "Setup.hpp"
#include <ByteBuffer.hpp>
#include <ObjectPool.hpp>
#include <demos/message.h>
#include <chrono>
#include <logging/gLog.h>
#include <print>
#include <build/_deps/boost-src/libs/asio/include/boost/asio/basic_socket_streambuf.hpp>
#include <build/_deps/boost-src/libs/asio/include/boost/asio/buffer.hpp>

TEST_F(BaseTest, ByteBuffer_validation_Allocate_Test)
{
    const auto some_string = "some string with special characters"
                             " 12345677 \u3000d !@#$%\n/n'\'n''''";
    gbase::ByteBuffer<std::byte> bb;
    bb.allocate(some_string, strlen(some_string));
    const auto buffer_ptr = bb.get();
    for (unsigned long i = 0; i < strlen(some_string); i++)
    {
        EXPECT_EQ(some_string[i], static_cast<char>(buffer_ptr.get()[i]));
    }

    bb.release();
}

TEST_F(BaseTest, ByteBuffer_validation_Append_Test)
{
    const auto some_string = "some string with special characters"
                             " 12345677 \u3000d !@#$%\n/n'\'n''''";
    gbase::ByteBuffer<std::byte> bb;
    bb.append(some_string, strlen(some_string));
    const auto buffer_ptr = bb.get();
    for (unsigned long i = 0; i < strlen(some_string); i++)
    {
        EXPECT_EQ(some_string[i], static_cast<char>(buffer_ptr.get()[i]));
    }

    // appending
    bb.append(some_string, strlen(some_string));
    const auto buffer_ptr_appended = bb.get();
    for (unsigned long i = 0; i < 2 * strlen(some_string); i++)
    {
        EXPECT_EQ(some_string[i % strlen(some_string)], static_cast<char>(buffer_ptr_appended.get()[i]));
    }

    // reallocate and ignore previous bytes
    const auto some_string_a = "abcef";
    bb.allocate(some_string_a, strlen(some_string_a));
    const auto buffer_ptr_reallocated = bb.get();
    for (unsigned long i = 0; i < strlen(some_string_a); i++)
    {
        EXPECT_EQ(some_string_a[i], static_cast<char>(buffer_ptr_reallocated.get()[i]));
    }

    bb.release();
}

TEST_F(BaseTest, ByteBuffer_validation_ByteBuffer_Append_Test)
{
    const auto some_string = "some string with special characters"
                             " 12345677 \u3000d !@#$%\n/n'\'n''''";
    auto *buffer = new std::byte[2048];
    for (size_t i = 0; i < strlen(some_string); i++)
    {
        buffer[i] = static_cast<std::byte>(*(some_string + i));
    }
    gbase::ByteBuffer<std::byte> bb;
    bb.append(buffer, strlen(some_string));
    const auto buffer_ptr = bb.get();
    for (unsigned long i = 0; i < strlen(some_string); i++)
    {
        EXPECT_EQ(some_string[i], static_cast<char>(buffer_ptr.get()[i]));
    }
    gbase::print_byte_array(bb);
    bb.release();
}

TEST_F(BaseTest, ByteBuffer_validation_ByteBuffer_Read_Write_Test)
{
    const Message message{8888, 1000, "Hello, Server request from clientele", true, 890};

    // serialize class into byte buffer
    gbase::ByteBuffer<std::byte> bb;
    message.serialize(bb);
    const auto buffer_ptr = bb.get();
    gbase::print_byte_array(bb);

    // reading int from an serialized class
    int int_value = 0;
    bb.read<int>(0, reinterpret_cast<char *>(&int_value));
    EXPECT_EQ(int_value, 8888);

    short short_value = 0;
    bb.read<short>(sizeof(Message::int_data), reinterpret_cast<char *>(&short_value));
    EXPECT_EQ(short_value, 1000);

    int string_length = 0;
    bb.read<int>(sizeof(Message::int_data) + sizeof(Message::short_data), reinterpret_cast<char *>(&string_length));
    auto buffer = std::make_unique<char[]>(string_length + 1);
    buffer[string_length] = '\0';
    bb.read(sizeof(Message::int_data) +
                sizeof(Message::short_data) + sizeof(Message::STRING_LENGTH),
            string_length, buffer.get());
    std::string strings;
    strings.assign(buffer.get());
    EXPECT_EQ(strings, message.string);
    buffer.reset();

    Message empty_message{};
    empty_message.deserialize(bb);

    std::cout << to_string(empty_message);
    bb.release();
}

TEST_F(BaseTest, ByteBuffer_validation_ByteBuffer_Copying_Test)
{
    const Message message{8888, 1000, "Hello, Server request from clientele", true, 890};

    // serialize class into byte buffer
    gbase::ByteBuffer<std::byte> bb;
    message.serialize(bb);

    std::vector<gbase::ByteBuffer<std::byte>> vec_;
    vec_.push_back(bb); // copied

    gbase::ByteBuffer<std::byte> bb_copied{vec_.at(0)};
    EXPECT_NE(&vec_[0], &bb);
    EXPECT_NE(&bb_copied, &bb);
    EXPECT_NE(&vec_[0], &bb_copied);
    bb_copied.release();
    vec_[0].release();
    bb.release();
}

TEST_F(BaseTest, ByteBuffer_validation_ByteBuffer_Moving_Test)
{
    const Message message{8888, 1000, "Hello, Server request from clientele", true, 890};

    // serialize class into byte buffer
    gbase::ByteBuffer<std::byte> bb;
    message.serialize(bb);

    std::vector<gbase::ByteBuffer<std::byte>> vec_;
    vec_.push_back(std::move(bb)); // movied

    gbase::ByteBuffer<std::byte> bb_moved{std::move(vec_.at(0))};
    EXPECT_NE(&vec_[0], &bb);
    EXPECT_NE(&bb_moved, &bb);
    EXPECT_NE(&vec_[0], &bb_moved);

    EXPECT_EQ(vec_[0].get(), nullptr);
    EXPECT_EQ(bb.get(), nullptr);

    EXPECT_EQ(vec_[0].get_buffer_size(), 0);
    EXPECT_EQ(bb.get_buffer_size(), 0);

    EXPECT_EQ(vec_[0].get_filled_size(), 0);
    EXPECT_EQ(bb.get_filled_size(), 0);

    gbase::print_byte_array(bb);
    gbase::print_byte_array(vec_[0]);
    gbase::print_byte_array(bb_moved);

    bb_moved.release(); // no issue
    vec_[0].release();  // no issue
    bb.release();       // no issue
}