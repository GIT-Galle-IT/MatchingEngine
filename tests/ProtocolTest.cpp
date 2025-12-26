#include "Setup.hpp"
#include <gProtocol.hpp>

TEST_F(BaseTest, protocol_testing)
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
