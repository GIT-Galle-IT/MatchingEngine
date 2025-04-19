#pragma once

#include <net/GSocket.h>
#include <net/Defs.h>

#include <utils/Common.h>

#include <map>
#include <queue>
#include <vector>
#include <sys/eventfd.h>

namespace GNet
{
    enum GServerMode : int
    {
        SYNC,
        ASYNC
    };

    class GServer
    {
    private:
        GSocket m_serverSocket;

        fd_set readfds;
        fd_set writefds;

        struct timeval tv;

        std::queue<std::string> incomingMsgBuffer;
        std::queue<std::string> outgoingMsgBuffer;

        G_EVENTFD eventNotifyingFileDiscriptor;

        std::vector<G_SOCKFD> m_clientSockets;
        GServerMode m_serverMode{GNet::GServerMode::SYNC};

        void startSyncLoop();
        void startAsyncLoop();

    public:
        GServer(GServerMode serverMode) : m_serverMode(serverMode) {}
        ~GServer() = default;

        int start(int port);
        bool closeClientConnection(G_SOCKFD clientsockfd);
        virtual void sendToClient(const std::string &data);

        virtual void onMessage(const std::string &request, std::string &response) {};
    };
} // namespace GServer
