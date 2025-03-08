#pragma once

#include "GSocket.h"

class GServer
{
private:
    GSocket m_server;
public:
    GServer() = default;
    ~GServer() = default;

    int start(int port);
    bool closeClientConnection(G_SOCKFD clientsockfd);

    virtual void onMessage(std::string& request, std::string& response){};
};


