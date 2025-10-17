// #include <utils/Common.h>
#include <gbase/net/GServer.h>
#include <sstream>
#include <iostream>
#include <gbase/logging/gLog.h>
#include "message.h"

int main()
{
    //  define server object
    gbase::net::GAsyncServer<> asyncServer; // at 8080
    asyncServer.init();
    asyncServer.start();

    // will not reach unless ctrl+c
    return 0;
}