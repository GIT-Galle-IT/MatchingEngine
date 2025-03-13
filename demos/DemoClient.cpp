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
        std::cout << "onResponse::  " << message << std::endl;
    }
};

int main()
{
    int i = 0;
    DemoClient client;
        client.connect("127.0.0.1", 9999);
    while (true)
    { 
        sleep(1);
        // creates client
        

        // create message
        i++;
        Message message{8888, 1000, "Hello, Server| this is an message", true, i};
        std::cout << "Size of request: " << sizeof(message) << std::endl;

        // serialize message (see DemoServer to see how to deserialize this message)
        std::stringstream oss;
        message.serialize(oss);
        std::cout << "Sending Message to server: " << message << std::endl;

        // send to the server
        auto serializedString = oss.str();
        client.send(serializedString);

        // close connection
        
    }
    client.closeConnection();
    return 0;
}