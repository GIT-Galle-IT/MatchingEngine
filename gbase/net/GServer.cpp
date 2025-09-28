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

    int maxfd = 0;
    eventfd_t holdingEvent = 0;
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
                GLOG("Client Connected")
                G_SOCKFD client = m_serverSocket.accept();
                m_clientSockets.push_back(client);
                continue;
            }

            if (FD_ISSET(eventNotifyingFileDiscriptor, &readfds) &&
                GNet::GServerMode::ASYNC == m_serverMode &&
                Event::NONE == static_cast<Event>(holdingEvent))
            {
                eventfd_read(eventNotifyingFileDiscriptor, &holdingEvent);
                GLOG("event read :- {}", holdingEvent);
            }

            int index = 0;
            for (auto client_fd : m_clientSockets)
            {
                ++index;
                if (FD_ISSET(client_fd, &readfds) == true)
                {
                    std::string request, response;
                    m_serverSocket.receiveData(client_fd, request);
                    GLOG("read from client {}", client_fd);
                    if (request.size() == 0)
                    {
                        GLOG("client {} closed connection", client_fd);
                        m_serverSocket.closeSocket(client_fd);
                        m_clientSockets.erase(m_clientSockets.begin() + index - 1);
                        continue;
                    }
                    if (GNet::GServerMode::SYNC == m_serverMode)
                    {
                        onMessage(request, response);
                        if (response.empty() == false)
                        {
                            m_serverSocket.sendData(client_fd, response);
                        }
                    }
                    else if (GNet::GServerMode::ASYNC == m_serverMode)
                    {
                        GLOG("queuing message to client {}", client_fd);
                        // incomingMsgBuffer.push(request); // send to processing thread
                        std::string response = "[ACK] response from server " + std::to_string(client_fd);
                        // m_serverSocket.sendData(client_fd, response);
                        send(client_fd, response);
                    }
                }
                if (FD_ISSET(client_fd, &writefds) == true &&
                    GNet::GServerMode::ASYNC == m_serverMode &&
                    Event::MESSAGE_BUFFERRED == static_cast<Event>(holdingEvent))
                {
                    GLOG("sending messages to client : {} | on event - {}", client_fd, holdingEvent);
                    auto it = outgoingMsgBuffer.find(client_fd); // replace with lock free queue
                    if (it == outgoingMsgBuffer.end() || it->second.empty() == true)
                        continue;
                    auto data = it->second.front();
                    if (data.empty() == false)
                        m_serverSocket.sendData(client_fd, data);
                    it->second.pop();
                }
            }
            holdingEvent = static_cast<eventfd_t>(Event::NONE);
        }
    }

    m_serverSocket.closeSelf();
}

void GServer::send(const G_SOCKFD &client, const std::string &data)
{
    // Cache the iterator to avoid repeated lookups
    auto it = outgoingMsgBuffer.find(client); // replace with lock free queue
    if (it != outgoingMsgBuffer.end())
    {
        it->second.push(data);
    }
    else
    {
        std::queue<std::string> q;
        q.push(data);
        auto result = outgoingMsgBuffer.emplace(client, std::move(q));
        it = result.first;
    }
    eventfd_write(eventNotifyingFileDiscriptor, static_cast<int>(Event::MESSAGE_BUFFERRED));
}