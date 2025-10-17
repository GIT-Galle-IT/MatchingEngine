// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include "message.h"

class DemoClient : public gbase::net::GSyncClient<std::stringstream>
{
public:
    virtual void onResponse(std::string &&message) override
    {
        // specify what to do upon recieving response
        GLOG_DEBUG_L1("reponse : {}", message);
    }
};

int main()
{
    int i = 0;
    std::unique_ptr<
        gbase::net::GClient<gbase::net::GEventHandlingMode::SYNC, std::stringstream>> 
            clientPtr = std::make_unique<DemoClient>();
    clientPtr->connect("127.0.0.1", 9999);
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

        clientPtr->send<std::stringstream>(std::move(oss));

        // close connection
        sleep(1);
    }
    return 0;
}