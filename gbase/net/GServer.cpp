#include "GServer.h"

int GServer::start(int port)
{
    
    if (!m_server.create() || !m_server.bind(port) || !m_server.listen(0))
    {
        std::cerr << "Server failed to start\n" << errno;
        return 1;
    }
    while (true)
    {
        G_SOCKFD client = m_server.accept();
        std::string request, response;
        if (client != -1)
        {
            m_server.receiveData(client, request);
            onMessage(request, response);
            m_server.sendData(client, response);
            m_server.closeSocket(client);
        }
    }
    
    // m_server.closeSelf();
}