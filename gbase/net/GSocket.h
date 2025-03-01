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
        return sockfd != -1;
    }

    bool bind(int port)
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        return ::bind(sockfd, (struct sockaddr *)&address, sizeof(address)) == 0;
    }

    bool listen(int backlog = 5)
    {
        return ::listen(sockfd, backlog) == 0;
    }

    G_SOCKFD accept()
    {
        socklen_t addrlen = sizeof(address);
        return ::accept(sockfd, (struct sockaddr *)&address, &addrlen);
    }

    bool connect(const char *ip, int port)
    {
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(ip);
        address.sin_port = htons(port);
        return ::connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == 0;
    }

    void sendData(G_SOCKFD recievengPartySocketfd, const char *data)
    {
        send(recievengPartySocketfd, data, strlen(data), 0);
    }

    void receiveData(char *buffer, size_t size)
    {
        recv(sockfd, buffer, size, 0);
    }

    void closeSocket(G_SOCKFD closingSocketFD)
    {
#ifdef _WIN32
        closesocket(closingSocketFD);
        WSACleanup();
#else
        close(closingSocketFD);
#endif
    }

    void closeSelf()
    {
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
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
