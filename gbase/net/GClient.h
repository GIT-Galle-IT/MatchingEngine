#pragma once

#include <utils/Common.h>
#include <GSocket.h>
#include <net/Defs.h>
#include <logging/gLog.h>
#include <type_traits>

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
        virtual void onResponse(const char *message) = 0;

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
        virtual void onResponse(const char *message) = 0;

        void send(T &ss) noexcept override
        {
            GLOG_DEBUG_L1("T& SS");
            std::string msg{std::move(ss.str())};
            this->clientSocket.sendData(msg);
            std::string response;
            this->clientSocket.receiveData(response);
            onResponse(response.c_str());
        };
        void send(const T &ss) noexcept override
        {
            GLOG_DEBUG_L1("const T& SS");
            std::string msg = "message"; // copy
            this->clientSocket.sendData(msg);
            std::string response;
            this->clientSocket.receiveData(response);
            onResponse(response.c_str());
        };
        void send(T &&ss) noexcept override
        {
            GLOG_DEBUG_L1("T&& SS");
            this->clientSocket.sendData(std::move(ss.str()));
            std::string response;
            this->clientSocket.receiveData(response);
            onResponse(response.c_str());
        };
        ;
    };

} // namespace gbase::net
