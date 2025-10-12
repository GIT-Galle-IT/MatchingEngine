#include <GServer.h>

using namespace gbase::net::l1;
using namespace gbase::net;
template<>
void gbase::net::GSyncServer<>::start()
{
    GLOG_DEBUG_L1("Sync Server loop started");
    while (true)
    {
        FD_ZERO(&writefds);
        FD_ZERO(&readfds);
        FD_SET(m_serverSocket.getSocketfd(), &readfds);
        int maxfd = m_serverSocket.getSocketfd();
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
        rv = select(maxfd + 1, &readfds, &writefds, NULL, &tv);
        if (rv != -1)
        {
            if (FD_ISSET(m_serverSocket.getSocketfd(), &readfds))
            {
                GLOG_DEBUG_L1("Client Connected")
                G_SOCKFD client = m_serverSocket.accept();
                m_clientSockets.push_back(client);
                continue;
            }

            int index = 0;
            for (auto client_fd : m_clientSockets)
            {
                ++index;
                if (FD_ISSET(client_fd, &readfds) == true)
                {
                    std::string request, response;
                    m_serverSocket.receiveData(client_fd, request);
                    GLOG_DEBUG_L1("read from client {}", client_fd);
                    if (request.size() == 0)
                    {
                        GLOG_DEBUG_L1("client {} closed connection", client_fd);
                        m_serverSocket.closeSocket(client_fd);
                        m_clientSockets.erase(m_clientSockets.begin() + index - 1);
                        continue;
                    }
                    onMessage(request, response);
                    if (response.empty() == false)
                    {
                        m_serverSocket.sendData(client_fd, response);
                    }
                }
            }
        }
    }
}

template<>
void gbase::net::GAsyncServer<>::start()
{
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
                GLOG_DEBUG_L1("Client Connected")
                G_SOCKFD client = m_serverSocket.accept();
                m_clientSockets.push_back(client);
                continue;
            }

            if (FD_ISSET(eventNotifyingFileDiscriptor, &readfds) &&
                gbase::net::GEventHandlingMode::ASYNC == m_serverMode &&
                Event::NONE == static_cast<Event>(holdingEvent))
            {
                eventfd_read(eventNotifyingFileDiscriptor, &holdingEvent);
                GLOG_DEBUG_L1("event read :- {}", holdingEvent);
            }

            int index = 0;
            for (auto client_fd : m_clientSockets)
            {
                ++index;
                if (FD_ISSET(client_fd, &readfds) == true)
                {
                    std::string request, response;
                    m_serverSocket.receiveData(client_fd, request);
                    GLOG_DEBUG_L1("read from client {}", client_fd);
                    if (request.size() == 0)
                    {
                        GLOG_DEBUG_L1("client {} closed connection", client_fd);
                        m_serverSocket.closeSocket(client_fd);
                        m_clientSockets.erase(m_clientSockets.begin() + index - 1);
                        continue;
                    }
                    GLOG_DEBUG_L1("queuing message to client {}", client_fd);
                    incomingMsgBuffer[client_fd].push(request);

#ifdef DEBUG
                    std::string response = "[ACK] response from server " + std::to_string(client_fd);
                    // m_serverSocket.sendData(client_fd, response);
                    send(client_fd, response);
#endif
                }
                if (FD_ISSET(client_fd, &writefds) == true &&
                    Event::MESSAGE_BUFFERRED == static_cast<Event>(holdingEvent))
                {
                    GLOG_DEBUG_L1("sending messages to client : {} | on event - {}", client_fd, holdingEvent);
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
}

template<>
void gbase::net::GAsyncServer<>::send(const G_SOCKFD &client, const std::string &data)
{
    // Cache the iterator to avoid repeated lookups
    auto it = outgoingMsgBuffer.find(client); // replace with lock free queue
    if (it != outgoingMsgBuffer.end())
    {
        it->second.push(data);
    }
    else
    {
        std::queue<std::string> messageQueue{};
        messageQueue.emplace(data);
        auto result = outgoingMsgBuffer.emplace(client, std::move(messageQueue));
    }
    eventfd_write(eventNotifyingFileDiscriptor, static_cast<int>(Event::MESSAGE_BUFFERRED));
}