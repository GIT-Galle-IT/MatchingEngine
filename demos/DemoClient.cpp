// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <iostream>

class DemoClient : public GClient
{
    public:
    DemoClient() = default;
    virtual void onResponse(const char* message) override
    {
        std::cout << "onResponse:::  " << message << std::endl;
    }
};

int main()
{
    GClient client;
    client.connect("127.0.0.1", 9999);
    client.send("Hello Server");
    return 0;
}