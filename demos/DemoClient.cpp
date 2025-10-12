// #include <utils/Common.h>
#include <gbase/net/GClient.h>
#include <sstream>
#include <iostream>
#include "message.h"

class MyString
{
public:
    MyString(std::string data) : data(data) {};
    MyString(const MyString &other) : data(other.data)
    {
        GLOG_INFO("MyString copy constructor called");
    }
    MyString(MyString &&other) noexcept : data(std::move(other.data))
    {
        GLOG_INFO("MyString move constructor called");
    }

    MyString &operator=(const MyString &other)
    {
        if (this != &other)
        {
            data = other.data;
            GLOG_INFO("MyString copy assignment operator called");
        }
        return *this;
    }
    MyString &operator=(MyString &&other) noexcept
    {
        if (this != &other)
        {
            data = std::move(other.data);
            GLOG_INFO("MyString move assignment operator called");
        }
        return *this;
    }

public:
    std::string data;
};

class DemoClient : public gbase::net::GSyncClient<MyString>
{
public:
    virtual void onResponse(const char *message) override
    {
        // specify what to do upon recieving response
        GLOG_DEBUG_L1("reponse : {}", message);
    }
};

int main()
{
    int i = 0;
    std::unique_ptr<
        gbase::net::GClient<gbase::net::GEventHandlingMode::SYNC, MyString>> 
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

        // send to the server
        MyString serializedString{oss.str()};
        // client.send<std::string>(std::move(serializedString));
        clientPtr->send<MyString>(std::move(serializedString));

        // close connection
        sleep(1);
    }
    return 0;
}