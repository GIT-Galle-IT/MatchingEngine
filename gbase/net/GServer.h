#pragma once

#include <GSocket.h>
#include <map>
#include <queue>
#include <vector>
#include <sys/eventfd.h>
#include <net/Defs.h>
#include <type_traits>
#include <utility>
#include <Defs.h>

namespace GNet
{
    template <GNet::GServerMode OperatingMode>
    class GServer
    {
    protected:
        GSocket m_serverSocket;

        fd_set readfds;
        fd_set writefds;

        struct timeval tv;

        static constexpr GServerMode m_serverMode{OperatingMode};
        std::vector<G_SOCKFD> m_clientSockets;

        int port;

    public:
        GServer(int port = 8080) : port(port) {};
        virtual ~GServer()
        {
            m_serverSocket.closeSelf();
        };

        GServer(GServer const &) = delete;
        GServer(GServer &&) = delete;
        GServer &operator=(GServer const &) = delete;
        GServer &operator=(GServer &&) = delete;

        void init()
        {
            if (!m_serverSocket.create())
            {
                GLOG_ERROR("Socket creation error {} {}", errno, strerror(errno));
                return;
            }

            int yes = static_cast<int>(YesNo::YES);
            if (setsockopt(m_serverSocket.getSocketfd(), SOL_SOCKET, SO_REUSEADDR,
                           (void *)(&yes), sizeof(yes)) < 0)
            {
                GLOG_ERROR("setsockopt() failed. {} {}", errno, strerror(errno));
                return;
            }

            if (!m_serverSocket.bind(port) || !m_serverSocket.listen(0))
            {
                GLOG_ERROR("Server failed to start {} {}", errno, strerror(errno));
                return;
            }
            GLOG_DEBUG_L1("Server started on port {}", port);
        };
    };

    template <GNet::GServerMode = GNet::GServerMode::ASYNC>
    class GAsyncServer : public GServer<GNet::GServerMode::ASYNC>
    {
    private:
        std::map<G_SOCKFD, std::queue<std::string>> incomingMsgBuffer;
        std::map<G_SOCKFD, std::queue<std::string>> outgoingMsgBuffer;

        G_EVENTFD eventNotifyingFileDiscriptor;

    public:
        GAsyncServer(int port = 8080) : GServer<GNet::GServerMode::ASYNC>::GServer(port) {};
        ~GAsyncServer() = default;

        GAsyncServer(GAsyncServer const &) = delete;
        GAsyncServer(GAsyncServer &&) = delete;
        GAsyncServer &operator=(GAsyncServer const &) = delete;
        GAsyncServer &operator=(GAsyncServer &&) = delete;

        void start();

        virtual void send(const G_SOCKFD &client, const std::string &data);
    };

    template <GNet::GServerMode = GNet::GServerMode::SYNC>
    class GSyncServer : public GServer<GNet::GServerMode::SYNC>
    {
    public:
        GSyncServer(int port = 8080) : GServer<GNet::GServerMode::SYNC>::GServer(port) {};
        ~GSyncServer() = default;

        GSyncServer(GSyncServer const &) = delete;
        GSyncServer(GSyncServer &&) = delete;
        GSyncServer &operator=(GSyncServer const &) = delete;
        GSyncServer &operator=(GSyncServer &&) = delete;

        void start();
        virtual void onMessage(const std::string &request, std::string &response) = 0;
    };
} // namespace GServer
