#include "GClient.h"

int GClient::connect(const char *ip, int port)
{
    if (!client.create() || !client.connect(ip, port))
    {
        GLOG_ERROR("Client failed to connect {} {}", errno, strerror(errno));
        return -1;
    }
    return 0;
}

void GClient::closeConnection()
{
    client.closeSelf();
}

void GClient::asyncSend(std::string& message)
{
    client.sendData(message);
}

void GClient::send(std::string& message)
{
    client.sendData(message);
    std::string response;
    client.receiveData(response);
    onResponse(response.c_str());
}
