#include <iostream>
#include <cstring>
#include <cstddef>
#include <logging/gLog.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sstream>
#endif

namespace gbase::net::l1
{
    using G_SOCKFD = int;
    using G_EVENTFD = int;

    class GSocket
    {
    private:
        G_SOCKFD sockfd;
        struct sockaddr_in address;

#ifdef _WIN32
        WSADATA wsa;
#endif

    public:
        GSocket() : sockfd(-1)
        {
#ifdef _WIN32
            WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        }

        [[gnu::always_inline]] inline G_SOCKFD getSocketfd() const noexcept
        {
            return sockfd;
        }

        bool create()
        {
            // onload / direct
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            GLOG_DEBUG_L1("Creating socket");
            return sockfd != -1;
        }

        bool bind(int port)
        {
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);
            GLOG_DEBUG_L1("Binding port {}", port);
            return ::bind(sockfd, (struct sockaddr *)&address, sizeof(address)) == 0;
        }

        bool listen(int backlog = 5)
        {
            GLOG_DEBUG_L1("now listening...");
            return ::listen(sockfd, backlog) == 0;
        }

        G_SOCKFD accept() noexcept
        {
            socklen_t addrlen = sizeof(address);
            auto ret = ::accept(sockfd, (struct sockaddr *)&address, &addrlen);
            GLOG_DEBUG_L1("Client connected {}:{}", inet_ntoa(address.sin_addr), address.sin_port);
            return ret;
        }

        bool connect(const char *ip, int port) noexcept
        {
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = inet_addr(ip);
            address.sin_port = htons(port);
            GLOG_DEBUG_L1("Connecting to {}:{}", ip, port);
            return ::connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == 0;
        }

        void sendData(G_SOCKFD recievengPartySocketfd, std::string &data) const
        {
            auto n = ::send(recievengPartySocketfd, data.c_str(), data.size(), 0);
            GLOG_DEBUG_L1("sent {} bytes", n);
        }

        void sendData(G_SOCKFD recievengPartySocketfd, std::string &&data) const
        {
            auto n = ::send(recievengPartySocketfd, data.c_str(), data.size(), 0);
            GLOG_DEBUG_L1("sent {} bytes", n);
        }

        // TODO: ERROR HANDLING
        void receiveData(G_SOCKFD clientSocketfd, std::string &data)
        {
            char buffer[2048];
            ssize_t readBytes;
            std::stringstream ss;
            int flag;
            do
            {
                readBytes = recv(clientSocketfd, buffer, sizeof(buffer), 0);
                if (readBytes > 0)
                {
                    buffer[readBytes] = '\0';
                    GLOG_DEBUG_L1("read {} bytes", readBytes);
                    ss.write(buffer, readBytes);
                    ioctl(clientSocketfd, FIONREAD, &flag);
                    buffer[0] = '\0';
                }
                else
                {
                    GLOG_DEBUG_L1("read {} bytes, connection closed by peer", readBytes);
                    flag = 0;
                }
            } while (flag > 0);
            data.assign(std::move(ss.str()));
        }

        void sendData(std::string &data)
        {
            sendData(sockfd, data);
        }

        void sendData(std::string &&data)
        {
            sendData(sockfd, data);
        }

        void receiveData(std::string &data)
        {
            receiveData(sockfd, data);
        }

        void closeSocket(G_SOCKFD closingSocketFD) noexcept
        {
#ifdef _WIN32
            closesocket(closingSocketFD);
            WSACleanup();
#else
            close(closingSocketFD);
            GLOG_DEBUG_L1("Closing client connection...");
#endif
        }

        void closeSelf() noexcept
        {
#ifdef _WIN32
            closesocket(sockfd);
            WSACleanup();
#else
            close(sockfd);
            GLOG_DEBUG_L1("Closing connection...");
#endif
        }
    };
} // namespace gbase::net

/*
// Server
int main()
{
    GSocket server;
    if (!server.create() || !server.bind(8080) || !server.listen())
    {
        std::cerr << "Server failed to start\n";
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";
    int client = server.accept();

    if (client != -1)
    {
        std::cout << "Client connected!\n";
        server.sendData("Hello, Client!");
    }

    server.closeSocket();
}

// client
int main()
{
    GSocket client;
    if (!client.create() || !client.connect("127.0.0.1", 8080))
    {
        std::cerr << "Client failed to connect\n";
        return 1;
    }

    char buffer[1024] = {0};
    client.receiveData(buffer, sizeof(buffer));
    std::cout << "Received from server: " << buffer << std::endl;

    client.closeSocket();
}
*/
