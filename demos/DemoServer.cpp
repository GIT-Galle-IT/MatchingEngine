// #include <utils/Common.h>
#include <gbase/net/GServer.h>
#include <iostream>

class DemoServer : public GServer
{
    virtual void onMessage(std::string& response) override
    {

        response = "Hi, Client";
    }
};

int main()
{
    DemoServer server;
    server.start(9999);
    return 0;
}