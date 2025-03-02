#include <iostream>
#include <cstring>

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

using G_SOCKFD = int;

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

    bool create()
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        printf("Creating socket\n");
        return sockfd != -1;
    }

    bool bind(int port)
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        printf("Binding port %d\n", port);
        return ::bind(sockfd, (struct sockaddr *)&address, sizeof(address)) == 0;
    }

    bool listen(int backlog = 5)
    {
        printf("now listening...\n");
        return ::listen(sockfd, backlog) == 0;
    }

    G_SOCKFD accept()
    {
        socklen_t addrlen = sizeof(address);
        auto ret = ::accept(sockfd, (struct sockaddr *)&address, &addrlen);
        printf("Client connected %s:%d\n", inet_ntoa(address.sin_addr), address.sin_port);
        return ret;
    }

    bool connect(const char *ip, int port)
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip);
        address.sin_port = htons(port);
        printf("Connecting to %s:%d\n", ip, port);
        return ::connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == 0;
    }

    void sendData(G_SOCKFD recievengPartySocketfd, std::string& data)
    {
        auto n = send(recievengPartySocketfd, data.c_str(), data.size(), 0);
        printf("sent %ld bytes\n", n);
    }

    void sendData(G_SOCKFD recievengPartySocketfd, std::stringstream& data)
    {
        const char* _ = data.str().c_str();
        auto n = send(recievengPartySocketfd, _, strlen(_), 0);
        printf("sent %ld bytes\n", n);
    }

    void receiveData(G_SOCKFD clientSocketfd, std::string& data)
    {
        char buffer[1024];
        ssize_t n;
        int flag;
        do
        {
            n = recv(clientSocketfd, buffer, sizeof(buffer), 0);
            if (n > 0)
            {
                buffer[n] = '\0';
                printf("read %ld bytes\n", n);
                for (size_t i = 0; i < n; i++)
                {
                    std::string c(1, buffer[i]); // copy byte by byte (find more effcient way)
                    data.append(c);
                }
                ioctl(clientSocketfd, FIONREAD, &flag);
                buffer[0] = '\0';
            }
            else
            {
                flag = 0;
            }
        } while (flag > 0);
    }

    void sendData(std::string& data)
    {
        sendData(sockfd, data);
    }

    void receiveData(std::string& data)
    {
        receiveData(sockfd, data);
    }

    

    void closeSocket(G_SOCKFD closingSocketFD)
    {
#ifdef _WIN32
        closesocket(closingSocketFD);
        WSACleanup();
#else
        close(closingSocketFD);
        printf("Closing client connection...\n");
#endif
    }

    void closeSelf()
    {
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
        printf("Closing connection...\n");
#endif
    }
};

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
