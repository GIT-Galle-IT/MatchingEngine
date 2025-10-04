// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include "message.h"

class DemoClient : public GClient
{
public:
    virtual void onResponse(const char *message) override
    {
        // specify what to do upon recieving response
        GLOG("onResponse : {}", message);
    }
};

int main()
{
    int i = 0;
    DemoClient client;
    client.connect("127.0.0.1", 9999);
    while (true)
    {

        // create message
        i++;
        Message message{8888, 1000, "Hello, Server request from clientele", true, i};
        GLOG("Size of request: {}", sizeof(message));

        // serialize message (see DemoServer to see how to deserialize this message)
        std::stringstream oss;
        message.serialize(oss);
        std::cout << "Sending Message to server: " << message << std::endl;
        GLOG("Serialized string size: {}", oss.str().size());
        GLOG("Serialized string: {}", oss.str());

        // send to the server
        auto serializedString = oss.str();
        client.send(serializedString);

        // close connection
        sleep(1);
    }
    client.closeConnection();
    return 0;
}