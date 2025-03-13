#include "GServer.h"

int GServer::start(int port)
{
    FD_ZERO(&writefds);
    FD_ZERO(&readfds);

    if (!m_server.create())
    {
        std::cerr << "Server failed to start\n"
                  << errno;
        return 1;
    }

    int yes = 1;
    if (setsockopt(m_server.getSocketfd(), SOL_SOCKET, SO_REUSEADDR,
                   (void *)&yes, sizeof(yes)) < 0)
    {
        fprintf(stderr, "setsockopt() failed. (%d)\n", errno);
    }

    if (!m_server.bind(port) || !m_server.listen(0))
    {
        std::cerr << "Server failed to start\n"
                  << errno;
        return 1;
    }

    int maxfd = 0;

    std::string request, response;
    while (true)
    {
        FD_ZERO(&writefds);
        FD_ZERO(&readfds);
        FD_SET(m_server.getSocketfd(), &readfds);
        for (auto clientVsReadyToCloseStat : clientSockets)
        {
            FD_SET(clientVsReadyToCloseStat.first, &readfds);
            FD_SET(clientVsReadyToCloseStat.first, &writefds);
            if (clientVsReadyToCloseStat.first > maxfd)
                maxfd = clientVsReadyToCloseStat.first;
        }

        if (maxfd == 0)
        {
            maxfd = m_server.getSocketfd();
            
        }
        // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
        tv.tv_sec = 10;
        tv.tv_usec = 500000;
        auto rv = select(maxfd + 1, &readfds, &writefds, 0, &tv);
        if (rv != -1)
        {
            if (FD_ISSET(m_server.getSocketfd(), &readfds))
            {
                G_SOCKFD client = m_server.accept();
                FD_SET(client, &readfds);
                FD_SET(client, &writefds);
                clientSockets[client] = false;
                continue;
            }
            auto copy = clientSockets;
            for (auto clientVsReadyToCloseStat : copy)
            {
                int taskCountOnClient = 0;
                if (FD_ISSET(clientVsReadyToCloseStat.first, &readfds))
                {
                    std::cout << "fdset true for read=" << rv << " client=" << clientVsReadyToCloseStat.first << std::endl;
                    request.clear();
                    m_server.receiveData(clientVsReadyToCloseStat.first, request);
                    onMessage(request, response);
                    clientVsReadyToCloseStat.second |= true;
                }
                if (FD_ISSET(clientVsReadyToCloseStat.first, &writefds))
                {
                    if (response.empty() == false)
                    {
                        m_server.sendData(clientVsReadyToCloseStat.first, response);
                        clientVsReadyToCloseStat.second |= false;
                    }
                    response.clear();
                }
                // if (clientVsReadyToCloseStat.second)
                // {
                //     std::cout << "Closing socket = " << clientVsReadyToCloseStat.first << std::endl;
                //     m_server.closeSocket(clientVsReadyToCloseStat.first);
                //     FD_CLR(clientVsReadyToCloseStat.first, &readfds);
                //     FD_CLR(clientVsReadyToCloseStat.first, &writefds);
                //     clientSockets.erase(clientVsReadyToCloseStat.first);
                //     maxfd = 0;
                // }
            }

            std::cout << "continuing\n\n\n\n"
                      << std::endl;
        }
    }

    // m_server.closeSelf();
}