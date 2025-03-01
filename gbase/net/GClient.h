#pragma once

#include <utils/Common.h>
#include "GSocket.h"

class GClient
{
private:
    /* data */
public:
    GClient() = default;
    ~GClient() = default;

    int connect(const char* ip, int port);
    void send(const char* message);

    virtual void onResponse(const char* message){};
};
