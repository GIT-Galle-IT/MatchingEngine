// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include <memory>
#include "message.h"

class DemoClient : public gbase::net::GAsyncClient<gbase::ByteBuffer<std::byte>>
{
};

int main()
{
    // std::unique_ptr<
    //     gbase::net::GClient<gbase::net::GEventHandlingMode::ASYNC, gbase::ByteBuffer<std::byte>>>
    //     clientPtr = std::make_unique<gbase::net::GAsyncClient<gbase::ByteBuffer<std::byte>>>();
    // clientPtr->connect("127.0.0.1", 8080);

    // std::thread clientThread([&clientPtr]()
    //                          {
    //     gbase::net::GAsyncClient<gbase::ByteBuffer<std::byte>>* asyncClient = 
    //         static_cast<gbase::net::GAsyncClient<gbase::ByteBuffer<std::byte>>*>(clientPtr.get());
    //         asyncClient->start(); });

    // std::thread producerThread([&clientPtr]()
    //                            {
    //     int i = 0;
    //     gbase::net::GAsyncClient<gbase::ByteBuffer<std::byte>>* asyncClient = 
    //         static_cast<gbase::net::GAsyncClient<gbase::ByteBuffer<std::byte>>*>(clientPtr.get());
    //     while (true)
    //     {

    //         // create message
    //         i++;
    //         Message message{8888, 1000, "Hello, Server request from clientele", true, i};
    //         GLOG_DEBUG_L1("Size of request: {}", sizeof(message));

    //         // serialize message (see DemoServer to see how to deserialize this message)
    //         gbase::ByteBuffer<std::byte> bb;
    //         message.serialize(bb);
    //         GLOG_DEBUG_L1("Serialized message: {}", to_string(message));

    //         // send to the server
    //         asyncClient->send<gbase::ByteBuffer<std::byte>>(std::move(bb));

    //         // close connection
    //         usleep(1000000);
    //     } });
    // clientThread.join();
    // producerThread.join();

    return 0;
}