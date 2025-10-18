// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include <memory>
#include "message.h"

class DemoClient : public gbase::net::GAsyncClient<std::stringstream>
{
};

int main()
{
    std::unique_ptr<
        gbase::net::GClient<gbase::net::GEventHandlingMode::ASYNC, std::stringstream>>
        clientPtr = std::make_unique<gbase::net::GAsyncClient<std::stringstream>>();
    clientPtr->connect("127.0.0.1", 8080);

    std::thread clientThread([&clientPtr]()
                             {
                                GLOG_INFO("Starting async client event loop...");
        gbase::net::GAsyncClient<std::stringstream>* asyncClient = 
            static_cast<gbase::net::GAsyncClient<std::stringstream>*>(clientPtr.get());
            asyncClient->start(); });
    
    std::thread producerThread([&clientPtr]()
                               {
        int i = 0;
        while (true)
        {

            // create message
            i++;
            Message message{8888, 1000, "Hello, Server request from clientele", true, i};
            GLOG_DEBUG_L1("Size of request: {}", sizeof(message));

            // serialize message (see DemoServer to see how to deserialize this message)
            std::stringstream oss;
            message.serialize(oss);
            GLOG_DEBUG_L1("Serialized message: {}", to_string(message));

            // send to the server
            clientPtr->send<std::stringstream>(std::move(oss));

            // close connection
            usleep(1000000);
        } });
        clientThread.join();
    producerThread.join();

    return 0;
}