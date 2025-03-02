// #include <utils/Common.h>
#include <gbase/net/GServer.h>
#include <sstream>
#include <iostream>
#include "message.h"

class DemoServer : public GServer
{
    private:
    Message message{0, 0, "0", 0, 0};

    virtual void onMessage(std::string& request, std::string& response) override
    {
        // specify what to do upon recieving request
        std::stringstream req(request);

        // deserilzie specified message to existing message object
        message.deserialize(req);
        std::cout << message << std::endl;
        
        response = "Ack Message";
    }
};

int main()
{
    //  define server object
    DemoServer server;
    
    //  start
    server.start(9999);

    // will not reach unless ctrl+c
    return 0;
}