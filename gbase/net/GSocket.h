#include <cstring>
#include <cstddef>
#include <logging/gLog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sstream>
#include <ByteBuffer.hpp>

namespace gbase::net::l1
{
    using G_SOCKETFD = int;
    using G_EVENTFD = int;

    class GSocket
    {
        G_SOCKETFD socket_fd;
        sockaddr_in address{};
        std::shared_ptr<ByteBuffer<std::byte>> send_buffer;
        std::shared_ptr<ByteBuffer<std::byte>> receive_buffer;

    public:
        GSocket() : socket_fd(-1)
        {
            send_buffer = std::make_shared<ByteBuffer<std::byte>>();
            receive_buffer = std::make_shared<ByteBuffer<std::byte>>();
        }

        [[nodiscard]] [[gnu::always_inline]] G_SOCKETFD getSocketFileDescriptor() const noexcept
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

        [[nodiscard]] bool listen(int backlog = 5) const
        {
            GLOG_DEBUG_L1("now listening...");
            return ::listen(socket_fd, backlog) == 0;
        }

        G_SOCKETFD accept() noexcept
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

        void send(const ByteBuffer<std::byte> &data) const
        {
            send(socket_fd, data);
        }

        void send(const ByteBuffer<std::byte> &&data) const
        {
            send(socket_fd, data);
        }

        static void send(const G_SOCKETFD receiving_party_socket_file_descriptor, const ByteBuffer<std::byte> &data)
        {
            send(receiving_party_socket_file_descriptor, std::move(data));
        }

        static void send(const G_SOCKETFD receiving_party_socket_file_descriptor, const ByteBuffer<std::byte> &&data)
        {
            auto n = ::send(receiving_party_socket_file_descriptor, data.get().get(), data.get_filled_size(), 0);
            GLOG_DEBUG_L1("sent {} bytes", n);
        }

        // TODO: ERROR HANDLING
        [[nodiscard]] auto receive(const G_SOCKETFD client_socket_file_descriptor) -> std::shared_ptr<ByteBuffer<std::byte>>
        {
            receive_buffer->release();
            std::byte buffer[2048];
            const std::stringstream ss;
            int flag;
            do
            {
                if (ssize_t readBytes = recv(client_socket_file_descriptor, buffer, sizeof(buffer), 0); readBytes > 0)
                {
                    GLOG_DEBUG_L1("read {} bytes", readBytes);
                    receive_buffer->append(buffer, readBytes);
                    ioctl(client_socket_file_descriptor, FIONREAD, &flag);
                    buffer[0] = static_cast<std::byte>('\0');
                }
                else
                {
                    GLOG_DEBUG_L1("read {} bytes, connection closed by peer", readBytes);
                    flag = 0;
                }
            } while (flag > 0);
            return receive_buffer;
        }

        [[nodiscard]] std::shared_ptr<ByteBuffer<std::byte>> receive()
        {
            return receive(socket_fd);
        }

        static void closeSocket(const G_SOCKETFD closingSocketFD) noexcept
        {
            close(closingSocketFD);
            GLOG_DEBUG_L1("Closing client connection...");
        }

        void closeSelf() const noexcept
        {
            close(socket_fd);
            GLOG_DEBUG_L1("Closing connection...");
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
        server.send("Hello, Client!");
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
    client.receive(buffer, sizeof(buffer));
    std::cout << "Received from server: " << buffer << std::endl;

    client.closeSocket();
}
*/
