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


namespace gbase::net
{
    using namespace gbase::net::l1;
    template <gbase::net::GEventHandlingMode T>
    class GServer
    {
    protected:
        GSocket m_serverSocket;

        fd_set readfds;
        fd_set writefds;

        struct timeval tv;

        static constexpr GEventHandlingMode m_serverMode{T};
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
                exit(1);
                return;
            }

            int yes = static_cast<int>(YesNo::YES);
            if (setsockopt(m_serverSocket.getSocketfd(), SOL_SOCKET, SO_REUSEADDR,
                           (void *)(&yes), sizeof(yes)) < 0)
            {
                GLOG_ERROR("setsockopt() failed. {} {}", errno, strerror(errno));
                exit(1);
                return;
            }

            if (!m_serverSocket.bind(port) || !m_serverSocket.listen(0))
            {
                GLOG_ERROR("Server failed to start {} {}", errno, strerror(errno));
                exit(1);
                return;
            }
            GLOG_DEBUG_L1("Server started on port {}", port);
        };
    };

    template <gbase::net::GEventHandlingMode = gbase::net::GEventHandlingMode::ASYNC>
    class GAsyncServer : public GServer<gbase::net::GEventHandlingMode::ASYNC>
    {
    private:
        std::map<G_SOCKFD, std::queue<std::string>> incomingMsgBuffer;
        std::map<G_SOCKFD, std::queue<std::string>> outgoingMsgBuffer;

        G_EVENTFD eventNotifyingFileDiscriptor;

    public:
        GAsyncServer(int port = 8080) : GServer<gbase::net::GEventHandlingMode::ASYNC>::GServer(port) {};
        ~GAsyncServer() = default;

        GAsyncServer(GAsyncServer const &) = delete;
        GAsyncServer(GAsyncServer &&) = delete;
        GAsyncServer &operator=(GAsyncServer const &) = delete;
        GAsyncServer &operator=(GAsyncServer &&) = delete;

        void start();

        virtual void send(const G_SOCKFD &client, const std::string &data);
    };

    template <gbase::net::GEventHandlingMode = gbase::net::GEventHandlingMode::SYNC>
    class GSyncServer : public GServer<gbase::net::GEventHandlingMode::SYNC>
    {
    public:
        GSyncServer(int port = 8080) : GServer<gbase::net::GEventHandlingMode::SYNC>::GServer(port) {};
        ~GSyncServer() = default;

        GSyncServer(GSyncServer const &) = delete;
        GSyncServer(GSyncServer &&) = delete;
        GSyncServer &operator=(GSyncServer const &) = delete;
        GSyncServer &operator=(GSyncServer &&) = delete;

        void start();
        virtual void onMessage(const std::string &request, std::string &response) = 0;
    };
} // namespace GServer
