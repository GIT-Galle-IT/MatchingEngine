#include <gbase/net/GServer.h>
#include <sstream>
#include <iostream>
#include <msg/message.h>

class MEServer : public GNet::GServer
{
public:
    MEServer() : GServer(GNet::GServerMode::SYNC) {}

private:
    OrderMessage message{"", 0, 0, 0};

    virtual void onMessage(const std::string &request, std::string &response) override
    {
        // specify what to do upon recieving request
        std::stringstream req(request);

        // deserilzie specified message to existing message object
        message.deserialize(req);
        // match
        std::cout << message << std::endl;

        response = "Ack Message";
    }
};

int main()
{
    //  define server object
    MEServer server;

    //  start
    server.start(9999);

    // will not reach unless ctrl+c
    return 0;
}
