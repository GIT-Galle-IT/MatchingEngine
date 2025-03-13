#pragma once

#include "GSocket.h"
#include <map>

class GServer
{
private:
    GSocket m_server;
    fd_set readfds;
    fd_set writefds;
    struct timeval tv;

    std::map<G_SOCKFD, bool /*ready to close or not*/> clientSockets;
public:
    GServer() = default;
    ~GServer() = default;

    int start(int port);
    bool closeClientConnection(G_SOCKFD clientsockfd);

    virtual void onMessage(std::string& request, std::string& response){};
};


