#include "GClient.h"

int GClient::connect(const char *ip, int port)
{
    GSocket client;
    if (!client.create() || !client.connect(ip, port))
    {
        std::cerr << "Client failed to connect\n";
        return 1;
    }

    char buffer[5] = {0};
    client.receiveData(buffer, sizeof(buffer));
    std::cout << "Received from server: " << buffer << std::endl;

    client.closeSelf();
    return 0;
}

void GClient::send(const char *message)
{
}
