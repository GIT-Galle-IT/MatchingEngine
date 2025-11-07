#include <GServer.h>

using namespace gbase::net::l1;
using namespace gbase::net;
template<>
void GSyncServer<>::start()
{
    GLOG_DEBUG_L1("Sync Server loop started");
    while (true)
    {
        FD_ZERO(&writefds);
        FD_ZERO(&readfds);
        FD_SET(m_serverSocket.getSocketFileDescriptor(), &readfds);
        int maxfd = m_serverSocket.getSocketFileDescriptor();
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
            if (FD_ISSET(m_serverSocket.getSocketFileDescriptor(), &readfds))
            {
                GLOG_DEBUG_L1("Client Connected")
                G_SOCKETFD client = m_serverSocket.accept();
                m_clientSockets.push_back(client);
                continue;
            }

            int index = 0;
            for (auto client_fd : m_clientSockets)
            {
                ++index;
                if (FD_ISSET(client_fd, &readfds) == true)
                {
                    std::string request {m_serverSocket.receiveData(client_fd)};
                    std::string response;
                    GLOG_DEBUG_L1("read from client {}", client_fd);
                    if (request.empty())
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
void GAsyncServer<>::start()
{
    int maxfd = 0;
    eventfd_t holdingEvent = 0;
    eventNotifyingFileDiscriptor = eventfd(0, EFD_SEMAPHORE);
    while (true)
    {
        FD_ZERO(&writefds);
        FD_ZERO(&readfds);
        FD_SET(m_serverSocket.getSocketFileDescriptor(), &readfds);
        FD_SET(eventNotifyingFileDiscriptor, &readfds);
        maxfd = eventNotifyingFileDiscriptor;
        for (const auto& client_fd : m_clientSockets)
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
            if (FD_ISSET(m_serverSocket.getSocketFileDescriptor(), &readfds))
            {
                GLOG_DEBUG_L1("Client Connected")
                G_SOCKETFD client = m_serverSocket.accept();
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
                    ByteBuffer<std::byte> bb;
                    bb.get().reset(m_serverSocket.receiveData());
                    GLOG_DEBUG_L1("read from client {}", client_fd);
                    if (request.empty())
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
                    const auto& byte_buffer = it->second.front();
                    GSocket::sendData(client_fd, byte_buffer);
                    it->second.pop();
                }
            }
            holdingEvent = static_cast<eventfd_t>(Event::NONE);
        }
    }
}

template<>
void GAsyncServer<>::send(const G_SOCKETFD &client, const ByteBuffer<std::byte> &data)
{
    // Cache the iterator to avoid repeated lookups
    if (const auto it = outgoingMsgBuffer.find(client); it != outgoingMsgBuffer.end())
    {
        it->second.push(data);
    }
    else
    {
        std::queue<ByteBuffer<std::byte>> messageQueue{};
        messageQueue.push(data);
        outgoingMsgBuffer.emplace(client, std::move(messageQueue));
    }
    eventfd_write(eventNotifyingFileDiscriptor, MESSAGE_BUFFERRED);
}