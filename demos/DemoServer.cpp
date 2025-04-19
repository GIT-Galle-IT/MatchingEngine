// #include <utils/Common.h>
#include <gbase/net/GServer.h>
#include <sstream>
#include <iostream>
#include "message.pb.cc"

class DemoServer : public GNet::GServer
{
public:
    DemoServer() : GServer(GNet::GServerMode::SYNC) {}

private:

    virtual void onMessage(const std::string &request, std::string &response) override
    {
        // specify what to do upon recieving request
        demos::AddressBook addressBook;
        addressBook.ParseFromString(request);
        std::cout << addressBook.people()[0].id() << std::endl;
        std::cout << addressBook.people()[0].name() << std::endl;
        std::cout << addressBook.people()[0].email() << std::endl;
        std::cout << addressBook.people()[0].phones()[0].number() << std::endl;
        google::protobuf::ShutdownProtobufLibrary();

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