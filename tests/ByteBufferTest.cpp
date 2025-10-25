// #include "../src/matching/MatchingEngine.hpp"
#include "Setup.hpp"
#include <ByteBuffer.hpp>
#include <demos/message.h>

TEST_F(BaseTest, ByteBuffer_Test)
{
    // MatchingEngine me{};
    // me.matchOrder();
    Message message{8888, 1000, "Hello, Server request from clientele", true, 10000000};
    GLOG_DEBUG_L1("Size of request: {}", sizeof(message));

    // serialize message (see DemoServer to see how to deserialize this message)
    std::stringstream oss;
    message.serialize(oss);
    const char *some_string = oss.str().c_str();
    ByteBuffer bb(some_string, oss.str().size());

    EXPECT_TRUE(true);
}
