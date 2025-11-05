#include <cstring>
#include <cstddef>
#include <logging/gLog.h>
#include <type_traits>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sstream>
#include "ByteBuffer.hpp"


namespace gbase::net::l1
{
    using G_SOCKFD = int;
    using G_EVENTFD = int;

    class GSocket
    {
        G_SOCKFD socket_fd;
        sockaddr_in address{};
        ByteBuffer<std::byte> send_buffer;
        ByteBuffer<std::byte> receive_buffer;

    public:
        GSocket() : socket_fd(-1) {}

        [[nodiscard]] [[gnu::always_inline]] G_SOCKFD getSocketFileDescriptor() const noexcept
        {
            return socket_fd;
        }

        bool create()
        {
            // onload / direct
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            GLOG_DEBUG_L1("Creating socket");
            return socket_fd != -1;
        }

        bool bind(int port)
        {
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);
            GLOG_DEBUG_L1("Binding port {}", port);
            return ::bind(socket_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == 0;
        }

        [[nodiscard]] bool listen(int backlog = 5) const {
            GLOG_DEBUG_L1("now listening...");
            return ::listen(socket_fd, backlog) == 0;
        }

        G_SOCKFD accept() noexcept
        {
            socklen_t addrlen = sizeof(address);
            const auto ret = ::accept(socket_fd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);
            GLOG_DEBUG_L1("Client connected {}:{}", inet_ntoa(address.sin_addr), address.sin_port);
            return ret;
        }

        bool connect(const char *ip, int port) noexcept
        {
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = inet_addr(ip);
            address.sin_port = htons(port);
            GLOG_DEBUG_L1("Connecting to {}:{}", ip, port);
            return ::connect(socket_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == 0;
        }

        static void sendData(const G_SOCKFD receiving_party_socket_file_descriptor, const std::string &data) {
            sendData(receiving_party_socket_file_descriptor, std::move(data));
        }

        static void sendData(const G_SOCKFD receiving_party_socket_file_descriptor, const std::string &&data) {
            auto n = ::send(receiving_party_socket_file_descriptor, data.c_str(), data.size(), 0);
            GLOG_DEBUG_L1("sent {} bytes", n);
        }

        // TODO: ERROR HANDLING
        auto receiveData(const G_SOCKFD client_socket_file_descriptor) -> std::string
        {
            std::byte buffer[2048];
            const std::stringstream ss;
            int flag;
            do
            {
                if (ssize_t readBytes = recv(client_socket_file_descriptor, buffer, sizeof(buffer), 0); readBytes > 0)
                {
                    GLOG_DEBUG_L1("read {} bytes", readBytes);
                    receive_buffer.append(buffer, readBytes);
                    ioctl(client_socket_file_descriptor, FIONREAD, &flag);
                    buffer[0] = static_cast<std::byte>('\0');
                }
                else
                {
                    GLOG_DEBUG_L1("read {} bytes, connection closed by peer", readBytes);
                    flag = 0;
                }
            } while (flag > 0);
            return ss.str();
        }

        void sendData(std::string &data) const
        {
            sendData(socket_fd, data);
        }

        void sendData(std::string &&data) const
        {
            sendData(socket_fd, data);
        }

        [[nodiscard]] std::string receiveData() const
        {
            return "receiveData(socket_fd)";
        }

        static void closeSocket(G_SOCKFD closingSocketFD) noexcept
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
            closesocket(socket_fd);
            WSACleanup();
#else
            close(socket_fd);
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
