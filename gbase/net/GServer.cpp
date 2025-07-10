#include "GServer.h"

using namespace GNet;

void GNet::GServer::startSyncLoop()
{
}

void GNet::GServer::startAsyncLoop()
{
}

int GServer::start(int port)
{
    if (!m_serverSocket.create())
    {
        std::cerr << "Server failed to start\n"
                  << errno;
        return 1;
    }

    int yes = 1;
    if (setsockopt(m_serverSocket.getSocketfd(), SOL_SOCKET, SO_REUSEADDR,
                   (void *)&yes, sizeof(yes)) < 0)
    {
        fprintf(stderr, "setsockopt() failed. (%d)\n", errno);
    }

    if (!m_serverSocket.bind(port) || !m_serverSocket.listen(0))
    {
        std::cerr << "Server failed to start\n"
                  << errno;
        return 1;
    }

    int maxfd = 0; eventfd_t holdingEvent = 0;
    eventNotifyingFileDiscriptor = eventfd(0, EFD_SEMAPHORE);

    while (true)
    {
        FD_ZERO(&writefds);
        FD_ZERO(&readfds);
        FD_SET(m_serverSocket.getSocketfd(), &readfds);
        FD_SET(eventNotifyingFileDiscriptor, &readfds);
        maxfd = eventNotifyingFileDiscriptor;
        for (auto client_fd : m_clientSockets)
        {
            FD_SET(client_fd, &readfds);
            FD_SET(client_fd, &writefds);
            if (client_fd > maxfd)
                maxfd = client_fd;
        }

        // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
        tv.tv_sec = 10;
        tv.tv_usec = 500000;
        int rv = -1;
        rv = select(maxfd + 1, &readfds, holdingEvent == Event::MESSAGE_BUFFERRED ? &writefds : NULL, NULL, &tv);
        if (rv != -1)
        {
            if (FD_ISSET(m_serverSocket.getSocketfd(), &readfds))
            {
                std::cout << "client connected" << std::endl;
                G_SOCKFD client = m_serverSocket.accept();
                m_clientSockets.push_back(client);
                continue;
            }

            if (GNet::GServerMode::ASYNC ==  m_serverMode &&
                FD_ISSET(eventNotifyingFileDiscriptor, &readfds) &&
                Event::NONE == static_cast<Event>(holdingEvent))
            {
                eventfd_read(eventNotifyingFileDiscriptor, &holdingEvent);
                std::cout << "event read : " << holdingEvent << std::endl;
                continue;
            }

            auto copyOfClients = m_clientSockets;
            int index = 0;
            for (auto client_fd : m_clientSockets)
            {
                ++index;
                if (FD_ISSET(client_fd, &readfds) == true)
                {
                    std::string request, response;
                    m_serverSocket.receiveData(client_fd, request);
                    if (request.size() == 0)
                    {
                        std::cout << client_fd << " : closed connection" << std::endl;
                        m_serverSocket.closeSocket(client_fd);
                        m_clientSockets.erase(m_clientSockets.begin()+index-1);
                        continue;
                    }
                    if (GNet::GServerMode::SYNC ==  m_serverMode)
                    {
                        onMessage(request, response);
                        if (response.empty() == false)
                        {
                            m_serverSocket.sendData(client_fd, response);
                        }
                    }
                    else if (GNet::GServerMode::ASYNC ==  m_serverMode)
                    {
                        incomingMsgBuffer.push(request); // send to processing thread
                        std::string response = "response from server";
                        // m_serverSocket.sendData(client_fd, response);
                        sendToClient(response);
                        std::cout << "buffer size " << incomingMsgBuffer.size() << std::endl;
                    }
                }
                if (GNet::GServerMode::ASYNC ==  m_serverMode &&
                    Event::MESSAGE_BUFFERRED == static_cast<Event>(holdingEvent) && 
                    FD_ISSET(client_fd, &writefds) == true)
                {
                    std::cout << "sending messages : " << outgoingMsgBuffer.size() << " | on event - " << holdingEvent << std::endl;
                    auto data = outgoingMsgBuffer.front();
                    if (data.empty() == false)
                    {
                        m_serverSocket.sendData(client_fd, data);
                    }
                    outgoingMsgBuffer.pop();
                    holdingEvent = static_cast<eventfd_t>(Event::NONE);
                }
            }
        }
    }

    m_serverSocket.closeSelf();
}

void GServer::sendToClient(const std::string& data)
{
    outgoingMsgBuffer.push(data); // replace with lock free queue
    eventfd_write(eventNotifyingFileDiscriptor, static_cast<int>(Event::MESSAGE_BUFFERRED));
}