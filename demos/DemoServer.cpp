// #include <utils/Common.h>
#include <gbase/net/GServer.h>
#include <sstream>
#include <iostream>
#include <gbase/logging/gLog.h>
#include "message.h"

class DemoServer : public GNet::GSyncServer<>
{
public:
    DemoServer(int port) : GNet::GSyncServer<>(port) {};
    ~DemoServer() = default;
    void onMessage(const std::string &request, std::string &response) override
    {
        // specify what to do upon recieving request
        std::stringstream req(request);

        // deserilzie specified message to existing message object
        message.deserialize(req);
        // match
        GLOG_DEBUG_L1("Received message : {}", to_string(message));

        response = "Ack Message";
    }

private:
    Message message{0, 0, "0", 0, 0};
};

int main()
{
    //  define server object
    DemoServer server(9999);
    GNet::GAsyncServer<> asyncServer;
    server.init();
    server.start();

    // will not reach unless ctrl+c
    return 0;
}