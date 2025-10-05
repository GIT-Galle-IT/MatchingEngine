#pragma once

#include <utils/Common.h>
#include "GSocket.h"

class GClient
{
private:
    GSocket client;
public:
    GClient() = default;
    ~GClient() = default;

    int connect(const char* ip, int port);
    void closeConnection();
    void send(std::string& message);

    virtual void onResponse(const char* message) = 0;
    // virtual void onResponse(std::istream& in_message){};
};
