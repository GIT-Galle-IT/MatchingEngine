#pragma once

#include <utils/Common.h>
#include <GSocket.h>
#include <net/Defs.h>
#include <logging/gLog.h>
#include <sys/eventfd.h>
#include <type_traits>
#include <memory>
#include <boost/lockfree/queue.hpp>

namespace gbase::net
{
    using namespace gbase::net::l1;
    template <GEventHandlingMode E, typename T>
        requires std::is_base_of_v<std::stringstream, T>
    class GClient
    {

    public:
        GClient() = default;
        virtual ~GClient() noexcept
        {
            close();
        };

        GClient(GClient const &) = delete;      // no copy
        GClient(GClient &&) = delete;           // no move
        GClient operator=(GClient &) = delete;  // no copy assignment
        GClient operator=(GClient &&) = delete; // no move assignment

        inline gbase::net::GEventHandlingMode getMode() const noexcept
        {
            return m_serverMode;
        }

        void connect(const char *ip, int port)
        {
            if (ip == nullptr || port <= 0)
            {
                GLOG_ERROR("Invalid IP address or port");
                exit(1);
                return;
            }

            if (!clientSocket.create() || !clientSocket.connect(ip, port))
            {
                GLOG_ERROR("Client failed to connect {} {}", errno, strerror(errno));
                exit(1);
                return;
            }
        }

        template <typename U = T>
        void send(T &&ss) noexcept
        {
            this->send(std::forward<U>(ss));
        }

        void close()
        {
            clientSocket.closeSelf();
        }

    protected:
        virtual void onResponse(std::string &&message) = 0;

        virtual void send(T &ss) noexcept = 0;
        virtual void send(const T &ss) noexcept = 0;
        virtual void send(T &&ss) noexcept = 0;

        static constexpr GEventHandlingMode m_serverMode{E};
        GSocket clientSocket;
    };

    template <typename T>
        requires std::is_base_of_v<std::stringstream, T>
    class GSyncClient : public GClient<GEventHandlingMode::SYNC, T>
    {
    public:
        GSyncClient() : GClient<GEventHandlingMode::SYNC, T>() {};
        virtual ~GSyncClient() = default;

        GSyncClient(GSyncClient const &) = delete;      // no copy
        GSyncClient(GSyncClient &&) = delete;           // no move
        GSyncClient operator=(GSyncClient &) = delete;  // no copy assignment
        GSyncClient operator=(GSyncClient &&) = delete; // no move assignment

    protected:
        virtual void onResponse(std::string &&message) = 0;

        void send(T &ss) noexcept override
        {
            this->clientSocket.sendData(ss.str());
            onResponse(this->clientSocket.receiveData());
        };

        void send(const T &ss) noexcept override
        {
            this->clientSocket.sendData(ss.str());
            onResponse(this->clientSocket.receiveData());
        };

        void send(T &&ss) noexcept override
        {
            this->clientSocket.sendData(std::move(ss.str()));
            onResponse(this->clientSocket.receiveData());
            ss.clear();
        };
        ;
    };

    template <typename T>
        requires std::is_base_of_v<std::stringstream, T>
    class GAsyncClient : public GClient<GEventHandlingMode::ASYNC, T>
    {
    public:
        GAsyncClient() : GClient<GEventHandlingMode::ASYNC, T>() {};
        virtual ~GAsyncClient() = default;

        GAsyncClient(GAsyncClient const &) = delete;      // no copy
        GAsyncClient(GAsyncClient &&) = delete;           // no move
        GAsyncClient operator=(GAsyncClient &) = delete;  // no copy assignment
        GAsyncClient operator=(GAsyncClient &&) = delete; // no move assignment

        void start() noexcept
        {
            int maxfd = 0;
            eventfd_t holdingEvent = 0;
            eventNotifyingFileDiscriptor = eventfd(0, EFD_SEMAPHORE);
            while (true)
            {
                FD_ZERO(&writefds);
                FD_ZERO(&readfds);
                FD_SET(this->clientSocket.getSocketfd(), &readfds);
                FD_SET(this->clientSocket.getSocketfd(), &writefds);
                FD_SET(eventNotifyingFileDiscriptor, &readfds);
                maxfd = eventNotifyingFileDiscriptor;

                // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
                tv.tv_sec = 10;
                tv.tv_usec = 500000;
                int rv = -1;
                rv = select(maxfd + 1, &readfds, holdingEvent == Event::MESSAGE_BUFFERRED ? &writefds : NULL, NULL, &tv);
                if (rv != -1)
                {

                    if (FD_ISSET(eventNotifyingFileDiscriptor, &readfds) &&
                        Event::NONE == static_cast<Event>(holdingEvent))
                    {
                        eventfd_read(eventNotifyingFileDiscriptor, &holdingEvent);
                        GLOG_DEBUG_L1("event read :- {}", holdingEvent);
                    }

                    if (FD_ISSET(this->clientSocket.getSocketfd(), &readfds) == true)
                    {
                        auto request = this->clientSocket.receiveData().c_str();
                        if (strlen(request) == 0)
                        {
                            this->clientSocket.closeSelf();
                            break;
                        }
                        incomingMsgQueue.push(request);
                    }

                    if (FD_ISSET(this->clientSocket.getSocketfd(), &writefds) == true &&
                        Event::MESSAGE_BUFFERRED == static_cast<Event>(holdingEvent))
                    {
                        if (outgoingMsgQueue.empty())
                        {
                            holdingEvent = static_cast<eventfd_t>(Event::NONE);
                            continue;
                        }
                        const char *data = nullptr;
                        auto hurray = outgoingMsgQueue.pop(data);
                        if (hurray)
                            this->clientSocket.sendData(data);
                    }
                }
            }
        }

        template <typename U = T>
        void send(T &&ss) noexcept
        {
            this->send(std::forward<U>(ss));
            eventfd_write(eventNotifyingFileDiscriptor, static_cast<int>(Event::MESSAGE_BUFFERRED));
        }

    protected:
        virtual void onResponse([[maybe_unused]] std::string &&message) override {};

        void send(T &ss) noexcept override
        {
            std::string str = ss.str(); // make a copy to ensure data validity
            incomingMsgQueue.push(str.c_str());
        };

        void send(const T &ss) noexcept override
        {
            std::string str = ss.str(); // make a copy to ensure data validity
            incomingMsgQueue.push(str.c_str());
        };

        void send(T &&ss) noexcept override
        {
            std::string str{ss.str()};
            incomingMsgQueue.push(str.c_str());
            ss.clear();
        };

    private:
        boost::lockfree::queue<const char *> incomingMsgQueue{1024};
        boost::lockfree::queue<const char *> outgoingMsgQueue{1024};

        fd_set readfds;
        fd_set writefds;

        struct timeval tv;

        eventfd_t eventNotifyingFileDiscriptor{0};
    };

} // namespace gbase::net
