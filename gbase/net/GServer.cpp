#include "GServer.h"

int GServer::start(int port)
{
    
    if (!m_server.create() || !m_server.bind(port) || !m_server.listen(0))
    {
        std::cerr << "Server failed to start\n" << errno;
        return 1;
    }
    std::cout << "Server listening on port " << port << "\n";
    while (true)
    {
        G_SOCKFD client = m_server.accept();
        if (client != -1)
        {
            std::cout << "Client connected!\n";
            std::string response;
            onMessage(response);
            std::cout << "response: " << response;
            m_server.sendData(client, response.c_str());
            m_server.closeSocket(client);
        }
    }
    
    // m_server.closeSelf();
}